<?php

require_once 'base/cli/Shell.php';
require_once 'srv/nrp/nrp.inc';
require_once 'pmm/fs/ModuleFS.php';
require_once 'base/utils/NetDetector.php';
require_once 'srv/nrp/xml/ProxyAdvanceXmlConfig.php';
require_once 'srv/nrp/xml/ProxyBackendXmlConfig.php';
require_once 'srv/nrp/xml/ProxyBackendVirtualPath.php';

define('NRP_PID_FILE_NAME','NRPd.pid');
define('PER_CONN_MEM', 50);		/** ÿ����ռ���ڴ��� 							(K)*/
define('MIN_FREE_MEM', 256);	/** ��Сʣ���ڴ������ȸ�ֵС���������½�����       (M) */

class NRPErrorDetector {

	public function parseError($errLine) {

	}
}

class MonitorBackend {
	const BACKEND_TYPE_BALANCE = 1;
	const BACKEND_TYPE_VIRTUALPATH = 2;
	const BACKEND_TYPE_VIRTUALDOMAINPORT = 3;

	public $type;
	public $virtualPath;
	public $frontendIp;
	public $frontendIpv6;
	public $frontendPort;
	public $frontendDomain;
	public $ip;
	public $port;
	public $status;

	public function __construct($type, $ip, $port, $virtualPath = null, $frontendIp = null, $frontendIpv6 = null, $frontendPort = null, $frontendDomain = null)  {
		$this->type = $type;
		$this->ip = $ip;
		$this->port = $port;
		$this->virtualPath = $virtualPath;
		$this->frontendIp = $frontendIp;
		$this->frontendIpv6 = $frontendIpv6;
		$this->frontendPort = $frontendPort;
		$this->frontendDomain = $frontendDomain;
	}
}

class NRPHelper {

	const NRP_ERROR_CODE = 10002;

	/** ͳ�Ƶ�ǰ����������	 */
	public static function countServiceCfg() {

		$module = ModuleFS::getModule("NRP");
		$moduleCfgDir = $module->getCfgPath();

		$cfgDirs = glob($moduleCfgDir . "/[0-9]*", GLOB_ONLYDIR);

		return count($cfgDirs);
	}

	/** ��ǰ���õ������*/
	public static function maxServiceCfgIndex() {

		$module = ModuleFS::getModule("NRP");
		$moduleCfgDir = $module->getCfgPath();

		$cfgDirs = glob($moduleCfgDir . "/[0-9]*", GLOB_ONLYDIR);

		$mx = 0;
		foreach ($cfgDirs as $dir) {
			$cfgIndex = basename($dir);
			if ($cfgIndex > $mx) {
				$mx = $cfgIndex;
			}
		}

		return $mx;
	}

	/**���ط�������Ŀ¼*/
	public static function getServiceCfgPath($id = null) {
		$module = ModuleFS::getModule("NRP");

		if ($id) {
			$moduleCfgDir = $module->getCfgPath($id);
		}
		else {
			$moduleCfgDir = $module->getCfgPath() . '/' . NRP_DEFAULT_CFG_NAME;
		}

		return $moduleCfgDir;
	}


	/**���ط�����־Ŀ¼*/
	public static function getServiceLogPath($id) {
		$module = ModuleFS::getModule("NRP");

		if ($id) {
			return $module->getLogPath($id);
		}
		else {
			return null;
		}
	}

	/**
	 * �жϺ�̨����ģʽ��WEB���أ�����·������·���˿ڣ���
	 */
	public static function isBalanceMode($id) {
		$configDir = NRPHelper::getServiceCfgPath($id);
		$backendCfg = new ProxyBackendXmlConfig($configDir);
		$mode = $backendCfg->getValue(BACKEND_MODE);
		return ($mode == "balance");
	}

	public static function isVirtualPathMode($id) {
		$configDir = NRPHelper::getServiceCfgPath($id);
		$backendCfg = new ProxyBackendXmlConfig($configDir);
		$mode = $backendCfg->getValue(BACKEND_MODE);
		return ($mode == "virtual-path");
	}

	public static function isVirtualDomainPortMode($id) {
		$configDir = NRPHelper::getServiceCfgPath($id);
		$backendCfg = new ProxyBackendXmlConfig($configDir);
		$mode = $backendCfg->getValue(BACKEND_MODE);
		return ($mode == "virtual-domain-port");
	}

	/**
	 * ���ؼ�����ַ�б�
	 */
	public static function getMonitorFrontends($cfgPath) {
		$monitorFrontends = array();
		//��˿���������
		$isVirtualDomainPort = NRPHelper::isVirtualDomainPortMode(basename($cfgPath));
		$isVirtualDomainPort = false;
		if ($isVirtualDomainPort) {
		}
		else {
			$basicParamCfg = new ProxyBasicXmlConfig($cfgPath);
			$currentIp = $basicParamCfg->getValue(BASIC_TAG_LOCAL_IP);
			$currentIpv6 = $basicParamCfg->getValue(BASIC_TAG_LOCAL_IPV6);
			$currentPort = $basicParamCfg->getValue(BASIC_TAG_LOCAL_PORT);
			$monitor_frontend = $currentIp . ':' . $currentPort;
			$monitor_frontend_v6 = '[' . $currentIpv6 . ']:' . $currentPort;
			if ($currentIp != "") {
				array_push($monitorFrontends, $monitor_frontend);
			}
			if ($currentIpv6 != "") {
				array_push($monitorFrontends, $monitor_frontend_v6);
			}
		}
		return $monitorFrontends;
	}

	/**
	 * ���غ�˽ڵ��б�
	 */
	public static function getMonitorBackends($cfgPath, $detect = true) {
		$basicParamCfg = new ProxyBasicXmlConfig($cfgPath);
		$service_status_port = $basicParamCfg->getValue(BASIC_TAG_SERVICE_PORT);
		$monitorBackends = array();
		$isBalance = NRPHelper::isBalanceMode(basename($cfgPath));
		$isVirtualPath = NRPHelper::isVirtualPathMode(basename($cfgPath));
		if ($isBalance) {
			$backendBalanceCfg = new ProxyBackendBalanceXmlConfig($cfgPath);
			$backends = $backendBalanceCfg->getBackends();
			foreach ($backends as $backend) {
				$monitor_backend = new MonitorBackend(MonitorBackend::BACKEND_TYPE_BALANCE, $backend->ip, $backend->port);
				if ($detect) {
					$monitor_backend->status = NRPHelper::getBackendStatus($backend->ip, $backend->port, true, "backend_", $service_status_port);
				} else {
					$monitor_backend = self::addDetectCfg($monitor_backend, $backend);
				}
				array_push($monitorBackends, $monitor_backend);
			}
		}
		else if($isVirtualPath) {
			$backendVirtualPathCfg = new ProxyBackendVirualPathXmlConfig($cfgPath);
			$backends = $backendVirtualPathCfg->getVirtualPathObjects();
			foreach ($backends as $backend) {
				foreach (explode(';', $backend->backend_host_port) as $item) {
					$item = str_replace(array('[', ']'), '', $item);
					$splitPosition = strripos($item, ':');
					$itemPort =  substr($item, $splitPosition + 1);
					$itemIP =  substr($item, 0, $splitPosition);
					$monitor_backend = new MonitorBackend(MonitorBackend::BACKEND_TYPE_VIRTUALPATH, $itemIP, $itemPort, $backend->virtualpath);
					$upstream_name = "backend" . str_replace("/", "_", $backend->virtualpath);
					if ($detect) {
						$monitor_backend->status = NRPHelper::getBackendStatus($itemIP, $itemPort, true, $upstream_name, $service_status_port);
					} else {
						$monitor_backend = self::addDetectCfg($monitor_backend, $backend);
					}
					array_push($monitorBackends, $monitor_backend);
				}
			}
		}
		else {
			$backendVirtualPathCfg = new ProxyBackendVirualPathXmlConfig($cfgPath);
			$backends = $backendVirtualPathCfg->getVirtualHostObjects();
			foreach ($backends as $backend) {
				$monitor_backend = new MonitorBackend(MonitorBackend::BACKEND_TYPE_VIRTUALDOMAINPORT, $backend->backend_host, $backend->backend_port, null, $backend->frontend_ip, $backend->frontend_ipv6, $backend->frontend_port, $backend->frontend_domain);
				$upstream_name = "backend_" . $backend->frontend_ip . "_" . str_replace(":", "-", $backend->frontend_ipv6) . "_" . $backend->frontend_port . "_" . str_replace(" ", "_", $backend->frontend_domain);
				if ($detect) {
					$monitor_backend->status = NRPHelper::getBackendStatus($backend->backend_host, $backend->backend_port, true, $upstream_name, $service_status_port);
				} else {
					$monitor_backend = self::addDetectCfg($monitor_backend, $backend);
				}
				array_push($monitorBackends, $monitor_backend);
			}
		}
		return $monitorBackends;
	}

	private static function addDetectCfg($monitor_backend, $backend)
    {
        $monitor_backend->detect_switch = $backend->detect_switch;
        $monitor_backend->detect_interval = $backend->detect_interval;
        $monitor_backend->detect_timeout = $backend->detect_timeout;
        $monitor_backend->detect_type = $backend->detect_type;
        $monitor_backend->detect_method = $backend->detect_method;
        $monitor_backend->detect_url = $backend->detect_url;
        $monitor_backend->detect_param = $backend->detect_param;
        $monitor_backend->detect_expect = $backend->detect_expect;
        return $monitor_backend;
    }

	/**
	 * ��ǰ���������
	 * �����½��������õ�·��
	 * */
	public static function createServiceCfg() {
		$newIndex = self::maxServiceCfgIndex() + 1;

		$cfgDir = self::getServiceCfgPath($newIndex);
		if (!mkdir($cfgDir, 0777, true)) {
			throw new Exception("����Ŀ¼" . $cfgDir . "ʧ��");
		}

		FileUtils::copy(NRP_MODULE_CFG_DIR . '/'.NRP_DEFAULT_CFG_NAME, $cfgDir);
		return $cfgDir;
	}

	public static function deleteServiceCfg($id) {
		if (trim($id) =='') {
			throw new Exception("������ɾ���������ڵķ���", self::NRP_ERROR_CODE);
		}

		$logDir = self::getServiceLogPath($id);
		$cfgDir = self::getServiceCfgPath($id);
		$cacheDir = NRP_DEFAULT_CACHE_PATH . '/' . $id;
		if (file_exists($cacheDir)) {
			$cmd = "sudo rm -rf $cfgDir/ $logDir/ $cacheDir/";
		} else {
			$cmd = "sudo rm -rf $cfgDir/ $logDir/";
		}
		try {
			_DEBUG("deleteServiceCfg  " . $cmd);
			ShellScript::execute($cmd);
		}
		catch(Exception $ex) {
			throw new Exception("ɾ������ʧ��", self::NRP_ERROR_CODE);
		}
	}

	/**
	 * ��������������Ŀ¼·��
	 */
	public static function getServiceCfgPaths() {
		$module = ModuleFS::getModule("NRP");
		$moduleCfgDir = $module->getCfgPath();

		$cfgDirs = glob($moduleCfgDir . "/[0-9]*", GLOB_ONLYDIR);

		//ȡ������idx,��������
		$indexs = array();
		foreach ($cfgDirs as $path) {
			$fileindex = basename($path);
			array_push($indexs, $fileindex);

			_DEBUG("getServiceCfgPaths found " . $fileindex . " " . $path);
		}
		sort($indexs);

		$retPaths = array();
		foreach ($indexs as $idx) {
			array_push($retPaths, $module->getCfgPath($idx));
		}

		return $retPaths;
	}

	public static function startService($id) {
		$module = ModuleFS::getModule("NRP");
		$module->setRedirectErr(1);
		try {
			$module->start($id);
		}
		catch(Exception $ex) {
			exec("tail -15 /var/log/NRP/".$id."/error.log | grep 'current engine is necessary, service is stopped'", $outputs, $ret);
			if ($ret == 0)
				throw new Exception("��������ʧ��:�����������ʧ��!������ܿ��豸������!", self::NRP_ERROR_CODE);
			else
				throw new Exception("��������ʧ��:������Ӧ��������Ϣ!", self::NRP_ERROR_CODE);
		}
	}

	public static function stopService($id, $force = false) {
		$module = ModuleFS::getModule("NRP");
		$module->setRedirectErr(0);
		try {
			$module->stop($id, $force);
		}
		catch (Exception $ex) {
			throw new Exception("ֹͣ����ʧ��:��鿴��Ӧ��������־��Ϣ!", self::NRP_ERROR_CODE);
		}
	}

	public static function startSksService($id, $force = false) {
		$module = ModuleFS::getModule("SKS");
		$module->setRedirectErr(0);
		try {
			$module->start($id, $force);
		}
		catch (Exception $ex) {
			throw new Exception("��������ʧ��,��鿴��Ӧ��������־��Ϣ!", self::NRP_ERROR_CODE);
		}
	}

	public static function stopSksService($id, $force = false) {
		$module = ModuleFS::getModule("SKS");
		$module->setRedirectErr(0);
		try {
			$module->stop($id, $force);
		}
		catch (Exception $ex) {
			throw new Exception("ֹͣ����ʧ��,��鿴��Ӧ��������־��Ϣ!", self::NRP_ERROR_CODE);
		}
	}

	public static function reloadService($id) {
		$module = ModuleFS::getModule("NRP");

		try {
			$module->reload($id);
		}
		catch(Exception $ex) {
			throw new Exception("��������ʧ��,��鿴��Ӧ��������־��Ϣ!",self::NRP_ERROR_CODE);
		}
	}

	public static function updateSnmpCacheData($id) {
		$module = ModuleFS::getModule("NRP");

		try {
			$module->updateSNMP($id);
		}
		catch(Exception $ex) {
			throw new Exception("����SNMP�����ļ�ʧ��,��鿴��Ӧ��������־��Ϣ!",self::NRP_ERROR_CODE);
		}
	}

	/**
	 * 	��ȡ����PID�ļ�
	 */
	public static function getServicePidFile($id) {
		$pidPath = NRP_DEFAULT_CACHE_PATH . '/' . $id;
		if (file_exists($pidPath)) {
			$servicePidFile = NRP_DEFAULT_CACHE_PATH . '/' . $id . '/' . NRP_PID_FILE_NAME;
		} else {
			$module = ModuleFS::getModule("NRP");
			$logDir = $module->getLogPath($id);
			$servicePidFile = $logDir . '/' . NRP_PID_FILE_NAME;
		}

		return $servicePidFile;
	}

	/**
	 * 	����PID������״̬
	 */
	public static function getServiceStatus($id) {
		$servicePidFile = self::getServicePidFile($id);

		if (file_exists($servicePidFile)) {
			$strPid = file_get_contents($servicePidFile);
			$strPid = trim($strPid);
			if ($strPid != "") {
				if(file_exists("/proc/$strPid")) {//�������Ƿ�����
					return true;
				}
			}
		}

		return false;
	}

	/**
	 * ��ȡ��������ʱ��
	 */
	public static function getServiceStartTime($id) {
		$servicePidFile = self::getServicePidFile($id);

		if (self::getServiceStatus($id)) {
			return filemtime($servicePidFile);
		}
		else {
			if (file_exists($servicePidFile)) {
				return filemtime($servicePidFile);
			}
		}

		return 0;
	}

	/**
	 * ��ȡ��˽ڵ�״̬�����ʼʱ��
	 */
	public static function getBackendStatusMonitorStartTime($id) {
		$backendStatusMonitorFile = NRP_DEFAULT_CACHE_PATH . "/" . $id . "/" . BACKEND_STATUS_MONITOR_FILE_NAME;
		if (file_exists($backendStatusMonitorFile)) {
			return filemtime($backendStatusMonitorFile);
		}

		return self::getServiceStartTime($id);
	}

	/**
	 * ��˽ڵ�״̬
	 */
	public static function getBackendStatus($ip, $port, $get_detail_status = false, $upstream_name = "backend_", $http_status_port = "", $http_status_ip = "127.0.0.1") {
		$detector = new HttpDetector();

		if(!$get_detail_status) {
			$ret = $detector->test($ip, $port, "/", "http");
			if ($ret) {
				return true;
			}
			else {
				return false;
			}
		} else {
			$status_array = array();
			if($http_status_port !== "") {
				if (filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6)) {
					$upstream_path = "/http_status/control?cmd=status&group=upstream@group&zone=" . $upstream_name . "@[" . $ip . "]:" . $port;
				}
				else {
					$upstream_path = "/http_status/control?cmd=status&group=upstream@group&zone=" . $upstream_name . "@" . $ip . ":" . $port;
				}
				$response = $detector->test($http_status_ip, $http_status_port, $upstream_path, "http", $get_detail_status);
				$response_header_end = strpos($response, "\r\n\r\n") + 4;
				$response_body = substr($response, $response_header_end);
				if($response_body !== "{}") {
					$status_array = json_decode($response_body, true);
				}
				return $status_array;
			} else {
				throw new Exception("��ȡ��˽ڵ���ϸ״̬����ָ��Status����˿ں�");
			}
		}
	}

	/**
	 * ���ú�˽ڵ�״̬
	 */
	public static function resetBackendStatus($id) {
		$cfgPath = NRPHelper::getServiceCfgPath($id);
		$basicParamCfg = new ProxyBasicXmlConfig($cfgPath);
		$service_status_port = $basicParamCfg->getValue(BASIC_TAG_SERVICE_PORT);
		$upstream_path = "/http_status/control?cmd=reset&group=upstream@group&zone=*";
		$detector = new HttpDetector();
		$ret = $detector->test("127.0.0.1", $service_status_port, $upstream_path, "http");
		if ($ret) {
			$backendStatusMonitorFile = NRP_DEFAULT_CACHE_PATH. "/" . $id . "/" . BACKEND_STATUS_MONITOR_FILE_NAME;
			touch($backendStatusMonitorFile);
			return true;
		}
		else {
			return false;
		}
	}

	public static function getLogDir($id) {
		$module = ModuleFS::getModule("NRP");
		return $module->getLogPath($id);
	}

	public static function getLogFilePath($id, $logType) {
		$logDir = NRPHelper::getLogDir($id);

		switch ($logType) {
		case "access":
			return $logDir . "/" . PROXY_ACCESS_LOG;
		case "error":
			return $logDir . "/" . PROXY_ERROR_LOG;
		default:
			return $logDir . "/" . PROXY_ERROR_LOG;
		}
	}

	/**
	 * ������˿��Ƿ����
	 */
	public static function checkPortAvailable($inputIp, $inputPort, $id) {

		if (empty($inputIp)) {
			return;
		}

		/*	����Ƿ���ϵͳ�����˿ڳ�ͻ	*/
		$reserved_ports = array(
			"21",  //ftp
			"22",  //ssh
			"23",  //telnet
			"53",  //dns
			"25",  //smtp
			"110", //pop3
			"113", //auth
			"115", //sftp
			"161", //snmp
			"162", //snmp-trap
			"389", //ldap
			"445", //microsoft-ds
			"500", //isakmp
			"873", //rsync
		);

		$sks_ports = array(
			"1880", //sks
			"18110", //sks
			"18443", //sks
			"18666", //sks
			"18888", //sks
			"18999", //sks
		);

		if (in_array($inputPort, $reserved_ports)) {
			throw new Exception("ָ���Ķ˿�" . $inputPort . "�ѱ�ϵͳ����");
		}

		/*	�������[192.168.190.7]���ȱ�ͬ��[4.4.4.x]��ռ��80�˿�	*/
		if($inputPort == 80 && ($inputIp == ALL_IP || $inputIp == "192.168.190.7" || strncmp($inputIp, "4.4.4.", 6) == 0)) {
			throw new Exception("ָ���Ķ˿�" . $inputIp . ":" . $inputPort . "�ѱ���������ʹ��");
		}

		if(in_array($inputPort, $sks_ports)) {
			throw new Exception("ָ���Ķ˿�" . $inputPort . "�ѱ�SKS����ռ��");
		}

		/*	����Ƿ�����������Ķ˿ڳ�ͻ	*/
		$cfgPaths = NRPHelper::getServiceCfgPaths();
		foreach ($cfgPaths as $cfgPath) {
			$idx = basename($cfgPath);

			//���뵱ǰ���ñȽϣ� ��Ҫ���ڸ��£�������������ö˿�
			if ($id == $idx) {
				continue;
			}

			$basicParamCfg = new ProxyBasicXmlConfig($cfgPath);
			$currentIp = $basicParamCfg->getValue(BASIC_TAG_LOCAL_IP);
			$currentIpv6 = $basicParamCfg->getValue(BASIC_TAG_LOCAL_IPV6);
			$currentPort = $basicParamCfg->getValue(BASIC_TAG_LOCAL_PORT);

			if (substr_count($inputIp, ":") > 1) { // inputIpΪIPv6��ַ
				if (($inputIp == ALL_IPV6_E && $currentIpv6 != '') ||
					($inputIp == ALL_IPV6_C && $currentIpv6 != '') ||
					($inputIp != '' && $currentIpv6 == ALL_IPV6_E) ||
					($inputIp != '' && $currentIpv6 == ALL_IPV6_C) ||
					($inputIp == $currentIpv6)) { // IPv6��ַƥ��
					if ($currentPort == $inputPort) {
						throw new Exception("ָ���Ķ˿�" . $inputIp . ":" . $inputPort . "�ѱ���������ʹ��");
					}
				}
			} else { // inputIpΪIPv4��ַ
				if (($inputIp == ALL_IP && $currentIp != '') ||
					($inputIp != '' && $currentIp == ALL_IP) ||
					($inputIp == $currentIp)) { // IPv4��ַƥ��
					if ($currentPort == $inputPort) {
						throw new Exception("ָ���Ķ˿�" . $inputIp . ":" . $inputPort . "�ѱ���������ʹ��");
					}
				}
			}

			$backendParamCfg = new ProxyBackendXmlConfig($cfgPath);
			$proxy_mode = $backendParamCfg->getValue(BACKEND_MODE);
			if($proxy_mode == "virtual-domain-port") {
				$backendVritualPathCfg = new ProxyBackendVirualPathXmlConfig($cfgPath);
				$virtualHosts = $backendVritualPathCfg->getVirtualHosts();
					foreach ($virtualHosts as $virtualHost) {
						$frontendIp = $virtualHost["frontend_ip"];
						$frontendIpv6 = $virtualHost["frontend_ipv6"];
						$frontendPort = $virtualHost["frontend_port"];
						$http_redirect_enable = $virtualHost["http_redirect_enable"];
						if (substr_count($inputIp, ":") > 1) { // inputIpΪIPv6��ַ
							if (($inputIp == ALL_IPV6_E && $frontendIpv6 != '') ||
								($inputIp == ALL_IPV6_C && $frontendIpv6 != '') ||
								($inputIp != '' && $frontendIpv6 == ALL_IPV6_E) ||
								($inputIp != '' && $frontendIpv6 == ALL_IPV6_C) ||
								($inputIp == $frontendIpv6)) { // IPv6��ַƥ��
								if ($currentPort == $inputPort) {
									throw new Exception("ָ���Ķ˿�" . $inputIp . ":" . $inputPort . "�ѱ���������ʹ��");
								}
								if ($http_redirect_enable == "on") {
									$http_redirect_port = $virtualHost["http_redirect_port"];
									if ($http_redirect_port == $inputPort) {
										throw new Exception("ָ���Ķ˿�" . $inputIp . ":" . $inputPort . "�ѱ���������ʹ��");
									}
								}
							}
						} else { // inputIpΪIPv4��ַ
							if (($inputIp == ALL_IP && $frontendIp != '') ||
								($inputIp != '' && $frontendIp == ALL_IP) ||
								($inputIp == $frontendIp)) { // IPv4��ַƥ��
								if ($currentPort == $inputPort) {
									throw new Exception("ָ���Ķ˿�" . $inputIp . ":" . $inputPort . "�ѱ���������ʹ��");
								}
								if ($http_redirect_enable == "on") {
									$http_redirect_port = $virtualHost["http_redirect_port"];
									if ($http_redirect_port == $inputPort) {
										throw new Exception("ָ���Ķ˿�" . $inputIp . ":" . $inputPort . "�ѱ���������ʹ��");
									}
								}
							}
						}
					}
			} else {
				$http_redirect_enable = $backendParamCfg->getValue(BACKEND_HTTP_REDIRECT_ENABLE);
				if($http_redirect_enable == "on") {
					$http_redirect_port = $backendParamCfg->getValue(BACKEND_HTTP_REDIRECT_PORT);
					if (substr_count($inputIp, ":") > 1) { // inputIpΪIPv6��ַ
						if (($inputIp == ALL_IPV6_E && $frontendIpv6 != '') ||
							($inputIp == ALL_IPV6_C && $frontendIpv6 != '') ||
							($inputIp != '' && $frontendIpv6 == ALL_IPV6_E) ||
							($inputIp != '' && $frontendIpv6 == ALL_IPV6_C) ||
							($inputIp == $frontendIpv6)) { // IPv6��ַƥ��
							if ($http_redirect_port == $inputPort) {
								throw new Exception("ָ���Ķ˿�" . $inputIp . ":" . $inputPort . "�ѱ���������ʹ��");
							}
						}
					} else { // inputIpΪIPv4��ַ
						if (($inputIp == ALL_IP && $frontendIp != '') ||
							($inputIp != '' && $frontendIp == ALL_IP) ||
							($inputIp == $frontendIp)) { // IPv4��ַƥ��
							if ($http_redirect_port == $inputPort) {
								throw new Exception("ָ���Ķ˿�" . $inputIp . ":" . $inputPort . "�ѱ���������ʹ��");
							}
						}
					}
				}
			}

		}
	}


	/**
	 * Ԥ��������Ҫռ�õ��ڴ�(K)
	 */
	private static function getServiceEstimateMemory($cfgPath) {
		$perfCfg = new ProxyPerfXmlConfig($cfgPath);
		$prcess_num = $perfCfg->getValue(TAG_WORKER_PROCESSES);
		$connection_nums = $perfCfg->getValue(TAG_WORKER_CONNECTIONS);
		$service_mem = PER_CONN_MEM * $prcess_num * $connection_nums; /** 50K * ������ * ÿ������������� */
		_DEBUG("���Ʒ���[" . $cfgPath . "]ÿ����ռ���ڴ�(K)[" . PER_CONN_MEM
				. "] ������[" .  $prcess_num . "]ÿ����������[" . $connection_nums
				. "]��ռ���ڴ�(K)" . $service_mem . "/Լ" . $service_mem / 1024 ."M");
		return  $service_mem;
	}

	/**
	 * Ԥ�����з���ռ�õ�����ڴ�(M)
	 */
	private static function getEstimateMaxMemory() {
		$cfgPaths = self::getServiceCfgPaths();
		$totalEstimateMem = 0;
		foreach ($cfgPaths as $cfgPath) {
			$totalEstimateMem += self::getServiceEstimateMemory($cfgPath);
		}
		_DEBUG("�������з�����ռ���ڴ�(K)[" . $totalEstimateMem . "]/Լ" . $totalEstimateMem / 1024 . "M");
		return $totalEstimateMem / 1024;
	}

	/**
	 * Ԥ��ʣ����ڴ�(M)
	 */
	public static function getEstimateFreeMemory() {
		$sys_mem = self::getMemoryCapacity();
		_DEBUG("ϵͳ���ڴ�(M):" . $sys_mem);
		$free_mem = $sys_mem - self::getEstimateMaxMemory();
		_DEBUG("����ʣ���ڴ�(M):" . $free_mem);
		return $free_mem;
	}

	/**
	 * ����Ƿ�����½�����
	 */
	public static function checkCreateService(){
		_DEBUG("getEstimateFreeMemory:" . self::getEstimateFreeMemory());
		if (self::getEstimateFreeMemory() < MIN_FREE_MEM) {
			_DEBUG("����ʣ���ڴ����ϵͳ�������" . MIN_FREE_MEM . "M, �����½�����");
			return false;
		}
		else {
			return true;
		}
	}

	/**
	 * ��ȡϵͳ�ڴ�����
	 */
	public static function getMemoryCapacity() {
		$cmd = 'head -n 1 /proc/meminfo | egrep -o "[0-9]+"';
		$ret = ShellScript::execute($cmd);

		/*	����λ��KBת��ΪMB	*/
		$capacity = intval($ret[0]) / 1024.0;

		return $capacity;
	}

	public static function getLogContent($file,$lines) {
		$cmd = " tail -n $lines $file";
		return ShellScript::execute($cmd);
	}

	public static function getCachePath($id) {
		$basicParamCfg = new ProxyPerfXmlConfig(NRP_MODULE_CFG_DIR ."/".$id);
		$dir = $basicParamCfg->getValue("request_temp_dir_prefix");
		if ($dir != "log") {
			$cachePath = NRP_DEFAULT_CACHE_PATH . '/' . $id . '/' . PROXY_CACHE_PATH;
		} else {
			$module = ModuleFS::getModule("NRP");
			$logDir = $module->getLogPath($id);
			$cachePath = $logDir . '/' . PROXY_CACHE_PATH;
		}
		return $cachePath;
	}

	public static function refreshCache($id) {
		$cachePath = self::getCachePath($id);
		$cmd = "sudo rm -rf $cachePath/*";
		try {
			_DEBUG($cmd);
			ShellScript::execute($cmd);
		}
		catch(Exception $ex) {
			throw new Exception("ǿ��ˢ�µ�ǰ���񻺴�ʧ�ܣ� index = $id");
		}
	}

}

?>
