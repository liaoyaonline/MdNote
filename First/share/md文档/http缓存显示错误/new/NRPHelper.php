<?php

require_once 'base/cli/Shell.php';
require_once 'srv/nrp/nrp.inc';
require_once 'pmm/fs/ModuleFS.php';
require_once 'base/utils/NetDetector.php';
require_once 'srv/nrp/xml/ProxyAdvanceXmlConfig.php';
require_once 'srv/nrp/xml/ProxyBackendXmlConfig.php';
require_once 'srv/nrp/xml/ProxyBackendVirtualPath.php';

define('NRP_PID_FILE_NAME','NRPd.pid');
define('PER_CONN_MEM', 50);		/** 每连接占用内存数 							(K)*/
define('MIN_FREE_MEM', 256);	/** 最小剩余内存数，比该值小将不允许新建服务       (M) */

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

	/** 统计当前服务配置数	 */
	public static function countServiceCfg() {

		$module = ModuleFS::getModule("NRP");
		$moduleCfgDir = $module->getCfgPath();

		$cfgDirs = glob($moduleCfgDir . "/[0-9]*", GLOB_ONLYDIR);

		return count($cfgDirs);
	}

	/** 当前配置的最大编号*/
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

	/**返回服务配置目录*/
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


	/**返回服务日志目录*/
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
	 * 判断后台代理模式（WEB负载？虚拟路径？多路径端口？）
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
	 * 返回监听地址列表
	 */
	public static function getMonitorFrontends($cfgPath) {
		$monitorFrontends = array();
		//多端口虚拟主机
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
	 * 返回后端节点列表
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
	 * 当前最大索引号
	 * 返回新建服务配置的路径
	 * */
	public static function createServiceCfg() {
		$newIndex = self::maxServiceCfgIndex() + 1;

		$cfgDir = self::getServiceCfgPath($newIndex);
		if (!mkdir($cfgDir, 0777, true)) {
			throw new Exception("创建目录" . $cfgDir . "失败");
		}

		FileUtils::copy(NRP_MODULE_CFG_DIR . '/'.NRP_DEFAULT_CFG_NAME, $cfgDir);
		return $cfgDir;
	}

	public static function deleteServiceCfg($id) {
		if (trim($id) =='') {
			throw new Exception("不允许删除，不存在的服务", self::NRP_ERROR_CODE);
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
			throw new Exception("删除服务失败", self::NRP_ERROR_CODE);
		}
	}

	/**
	 * 返回所有配置项目录路径
	 */
	public static function getServiceCfgPaths() {
		$module = ModuleFS::getModule("NRP");
		$moduleCfgDir = $module->getCfgPath();

		$cfgDirs = glob($moduleCfgDir . "/[0-9]*", GLOB_ONLYDIR);

		//取出服务idx,进行排序
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
				throw new Exception("启动服务失败:加密引擎加载失败!请检查加密卡设备及驱动!", self::NRP_ERROR_CODE);
			else
				throw new Exception("启动服务失败:请检查相应的配置信息!", self::NRP_ERROR_CODE);
		}
	}

	public static function stopService($id, $force = false) {
		$module = ModuleFS::getModule("NRP");
		$module->setRedirectErr(0);
		try {
			$module->stop($id, $force);
		}
		catch (Exception $ex) {
			throw new Exception("停止服务失败:请查看相应的运行日志信息!", self::NRP_ERROR_CODE);
		}
	}

	public static function startSksService($id, $force = false) {
		$module = ModuleFS::getModule("SKS");
		$module->setRedirectErr(0);
		try {
			$module->start($id, $force);
		}
		catch (Exception $ex) {
			throw new Exception("启动服务失败,请查看相应的运行日志信息!", self::NRP_ERROR_CODE);
		}
	}

	public static function stopSksService($id, $force = false) {
		$module = ModuleFS::getModule("SKS");
		$module->setRedirectErr(0);
		try {
			$module->stop($id, $force);
		}
		catch (Exception $ex) {
			throw new Exception("停止服务失败,请查看相应的运行日志信息!", self::NRP_ERROR_CODE);
		}
	}

	public static function reloadService($id) {
		$module = ModuleFS::getModule("NRP");

		try {
			$module->reload($id);
		}
		catch(Exception $ex) {
			throw new Exception("重启服务失败,请查看相应的运行日志信息!",self::NRP_ERROR_CODE);
		}
	}

	public static function updateSnmpCacheData($id) {
		$module = ModuleFS::getModule("NRP");

		try {
			$module->updateSNMP($id);
		}
		catch(Exception $ex) {
			throw new Exception("更新SNMP缓存文件失败,请查看相应的运行日志信息!",self::NRP_ERROR_CODE);
		}
	}

	/**
	 * 	获取服务PID文件
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
	 * 	根据PID检查服务状态
	 */
	public static function getServiceStatus($id) {
		$servicePidFile = self::getServicePidFile($id);

		if (file_exists($servicePidFile)) {
			$strPid = file_get_contents($servicePidFile);
			$strPid = trim($strPid);
			if ($strPid != "") {
				if(file_exists("/proc/$strPid")) {//检查进程是否启动
					return true;
				}
			}
		}

		return false;
	}

	/**
	 * 获取服务启动时间
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
	 * 获取后端节点状态监控起始时间
	 */
	public static function getBackendStatusMonitorStartTime($id) {
		$backendStatusMonitorFile = NRP_DEFAULT_CACHE_PATH . "/" . $id . "/" . BACKEND_STATUS_MONITOR_FILE_NAME;
		if (file_exists($backendStatusMonitorFile)) {
			return filemtime($backendStatusMonitorFile);
		}

		return self::getServiceStartTime($id);
	}

	/**
	 * 后端节点状态
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
				throw new Exception("获取后端节点详细状态必须指定Status服务端口号");
			}
		}
	}

	/**
	 * 重置后端节点状态
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
	 * 检查服务端口是否可用
	 */
	public static function checkPortAvailable($inputIp, $inputPort, $id) {

		if (empty($inputIp)) {
			return;
		}

		/*	检查是否与系统保留端口冲突	*/
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
			throw new Exception("指定的端口" . $inputPort . "已被系统保留");
		}

		/*	管理界面[192.168.190.7]和热备同步[4.4.4.x]已占用80端口	*/
		if($inputPort == 80 && ($inputIp == ALL_IP || $inputIp == "192.168.190.7" || strncmp($inputIp, "4.4.4.", 6) == 0)) {
			throw new Exception("指定的端口" . $inputIp . ":" . $inputPort . "已被其他服务使用");
		}

		if(in_array($inputPort, $sks_ports)) {
			throw new Exception("指定的端口" . $inputPort . "已被SKS服务占用");
		}

		/*	检查是否与其他服务的端口冲突	*/
		$cfgPaths = NRPHelper::getServiceCfgPaths();
		foreach ($cfgPaths as $cfgPath) {
			$idx = basename($cfgPath);

			//不与当前配置比较， 主要是在更新，不检查自身配置端口
			if ($id == $idx) {
				continue;
			}

			$basicParamCfg = new ProxyBasicXmlConfig($cfgPath);
			$currentIp = $basicParamCfg->getValue(BASIC_TAG_LOCAL_IP);
			$currentIpv6 = $basicParamCfg->getValue(BASIC_TAG_LOCAL_IPV6);
			$currentPort = $basicParamCfg->getValue(BASIC_TAG_LOCAL_PORT);

			if (substr_count($inputIp, ":") > 1) { // inputIp为IPv6地址
				if (($inputIp == ALL_IPV6_E && $currentIpv6 != '') ||
					($inputIp == ALL_IPV6_C && $currentIpv6 != '') ||
					($inputIp != '' && $currentIpv6 == ALL_IPV6_E) ||
					($inputIp != '' && $currentIpv6 == ALL_IPV6_C) ||
					($inputIp == $currentIpv6)) { // IPv6地址匹配
					if ($currentPort == $inputPort) {
						throw new Exception("指定的端口" . $inputIp . ":" . $inputPort . "已被其他服务使用");
					}
				}
			} else { // inputIp为IPv4地址
				if (($inputIp == ALL_IP && $currentIp != '') ||
					($inputIp != '' && $currentIp == ALL_IP) ||
					($inputIp == $currentIp)) { // IPv4地址匹配
					if ($currentPort == $inputPort) {
						throw new Exception("指定的端口" . $inputIp . ":" . $inputPort . "已被其他服务使用");
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
						if (substr_count($inputIp, ":") > 1) { // inputIp为IPv6地址
							if (($inputIp == ALL_IPV6_E && $frontendIpv6 != '') ||
								($inputIp == ALL_IPV6_C && $frontendIpv6 != '') ||
								($inputIp != '' && $frontendIpv6 == ALL_IPV6_E) ||
								($inputIp != '' && $frontendIpv6 == ALL_IPV6_C) ||
								($inputIp == $frontendIpv6)) { // IPv6地址匹配
								if ($currentPort == $inputPort) {
									throw new Exception("指定的端口" . $inputIp . ":" . $inputPort . "已被其他服务使用");
								}
								if ($http_redirect_enable == "on") {
									$http_redirect_port = $virtualHost["http_redirect_port"];
									if ($http_redirect_port == $inputPort) {
										throw new Exception("指定的端口" . $inputIp . ":" . $inputPort . "已被其他服务使用");
									}
								}
							}
						} else { // inputIp为IPv4地址
							if (($inputIp == ALL_IP && $frontendIp != '') ||
								($inputIp != '' && $frontendIp == ALL_IP) ||
								($inputIp == $frontendIp)) { // IPv4地址匹配
								if ($currentPort == $inputPort) {
									throw new Exception("指定的端口" . $inputIp . ":" . $inputPort . "已被其他服务使用");
								}
								if ($http_redirect_enable == "on") {
									$http_redirect_port = $virtualHost["http_redirect_port"];
									if ($http_redirect_port == $inputPort) {
										throw new Exception("指定的端口" . $inputIp . ":" . $inputPort . "已被其他服务使用");
									}
								}
							}
						}
					}
			} else {
				$http_redirect_enable = $backendParamCfg->getValue(BACKEND_HTTP_REDIRECT_ENABLE);
				if($http_redirect_enable == "on") {
					$http_redirect_port = $backendParamCfg->getValue(BACKEND_HTTP_REDIRECT_PORT);
					if (substr_count($inputIp, ":") > 1) { // inputIp为IPv6地址
						if (($inputIp == ALL_IPV6_E && $frontendIpv6 != '') ||
							($inputIp == ALL_IPV6_C && $frontendIpv6 != '') ||
							($inputIp != '' && $frontendIpv6 == ALL_IPV6_E) ||
							($inputIp != '' && $frontendIpv6 == ALL_IPV6_C) ||
							($inputIp == $frontendIpv6)) { // IPv6地址匹配
							if ($http_redirect_port == $inputPort) {
								throw new Exception("指定的端口" . $inputIp . ":" . $inputPort . "已被其他服务使用");
							}
						}
					} else { // inputIp为IPv4地址
						if (($inputIp == ALL_IP && $frontendIp != '') ||
							($inputIp != '' && $frontendIp == ALL_IP) ||
							($inputIp == $frontendIp)) { // IPv4地址匹配
							if ($http_redirect_port == $inputPort) {
								throw new Exception("指定的端口" . $inputIp . ":" . $inputPort . "已被其他服务使用");
							}
						}
					}
				}
			}

		}
	}


	/**
	 * 预估服务需要占用的内存(K)
	 */
	private static function getServiceEstimateMemory($cfgPath) {
		$perfCfg = new ProxyPerfXmlConfig($cfgPath);
		$prcess_num = $perfCfg->getValue(TAG_WORKER_PROCESSES);
		$connection_nums = $perfCfg->getValue(TAG_WORKER_CONNECTIONS);
		$service_mem = PER_CONN_MEM * $prcess_num * $connection_nums; /** 50K * 进程数 * 每进程最大连接数 */
		_DEBUG("估计服务[" . $cfgPath . "]每连接占用内存(K)[" . PER_CONN_MEM
				. "] 进程数[" .  $prcess_num . "]每进程连接数[" . $connection_nums
				. "]总占用内存(K)" . $service_mem . "/约" . $service_mem / 1024 ."M");
		return  $service_mem;
	}

	/**
	 * 预估所有服务占用的最大内存(M)
	 */
	private static function getEstimateMaxMemory() {
		$cfgPaths = self::getServiceCfgPaths();
		$totalEstimateMem = 0;
		foreach ($cfgPaths as $cfgPath) {
			$totalEstimateMem += self::getServiceEstimateMemory($cfgPath);
		}
		_DEBUG("估计所有服务总占用内存(K)[" . $totalEstimateMem . "]/约" . $totalEstimateMem / 1024 . "M");
		return $totalEstimateMem / 1024;
	}

	/**
	 * 预估剩余的内存(M)
	 */
	public static function getEstimateFreeMemory() {
		$sys_mem = self::getMemoryCapacity();
		_DEBUG("系统总内存(M):" . $sys_mem);
		$free_mem = $sys_mem - self::getEstimateMaxMemory();
		_DEBUG("估计剩余内存(M):" . $free_mem);
		return $free_mem;
	}

	/**
	 * 检查是否可以新建服务
	 */
	public static function checkCreateService(){
		_DEBUG("getEstimateFreeMemory:" . self::getEstimateFreeMemory());
		if (self::getEstimateFreeMemory() < MIN_FREE_MEM) {
			_DEBUG("估计剩余内存低于系统最低限制" . MIN_FREE_MEM . "M, 不能新建服务");
			return false;
		}
		else {
			return true;
		}
	}

	/**
	 * 获取系统内存总量
	 */
	public static function getMemoryCapacity() {
		$cmd = 'head -n 1 /proc/meminfo | egrep -o "[0-9]+"';
		$ret = ShellScript::execute($cmd);

		/*	将单位从KB转换为MB	*/
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
			throw new Exception("强制刷新当前服务缓存失败： index = $id");
		}
	}

}

?>
