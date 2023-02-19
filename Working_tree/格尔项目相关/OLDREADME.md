> 此页面用于汇总云网关项目的所有文档，包括需求，设计和预研

# 开发指南

- [文档维护规范](guide-docs/文档维护规范.md)
- [API设计风格指南](guide-docs/API设计风格指南.md)
- [代码-镜像-标签命名指南](guide-docs/代码-镜像-标签命名指南.md)
- [技术栈说明](guide-docs/技术栈指南.md)
- [交付件规范](guide-docs/交付件规范.md)
- [代理审查指南](guide-docs/代码审查指南.md)

# [智能网关管理平台](IGMP-README.md)

# 下一代安全网关高级需求

- [业务架构](docs/arch/业务架构.md)
  - [产品需求来源](docs/arch/产品需求来源.md)

- 业务模块
  - [TRP高级需求](docs/TRP代理功能设计.md)
  - [PPS高级需求](docs/pps/PPS高级需求.md)
  - [PPS需求文档](docs/pps/pps需求文档.md)
  - [TODO：VPN高级需求](docs/VPN高级需求.md)
  - [TODO：SSO高级需求](docs/SSO高级需求.md)
- 系统模块
  - [单机版网关界面：CONSOLE高级需求](docs/CONSOLE高级需求.md)
  - [策略管理服务：RMS高级需求](docs/RMS高级需求.md)
  - [TODO：权限引擎：PES高级需求](docs/PES高级需求.md)
  - [网关管理模块：NETMAN高级需求](docs/NETMAN高级需求.md)
- 内部模块
  - [应用健康检测：BDT高级需求](docs/BDT高级需求.md)
  - [DMS高级需求](docs/DMS高级需求.md)
  - [日志分析系统：LAM高级需求](docs/LAM高级需求.md)
  - [OCSP高级需求](docs/OCSP高级需求.md)
  - [API网关：KONG高级需求](docs/KONG高级需求.md)
  - [日志收集器：TELEGRAF高级需求](docs/TELEGRAF高级需求.md)
  - [日志数据库：INFLUX高级需求](docs/INFLUX高级需求.md)
  - [KAPACITOR高级需求](docs/KAPACITOR高级需求.md)
  - [TODO：CONFS高级需求](docs/CONFS高级需求.md)
  - [TODO：REDIS高级需求](docs/REDIS高级需求.md)
  - [TODO：POSTGRES高级需求](docs/POSTGRES高级需求.md)
- 非功能需求
  - [系统管理功能需求](docs/系统管理功能设计.md)
  - [可用性&可维护性需求](docs/可用性&可维护性需求.md)
  - [安装部署高级需求](docs/安装部署高级需求.md)
  

# 概要设计

- [UI设计](http://git.koal.com/gw-cloud/gateway-ui-desigin.md)

- 整体设计
  - [系统内部架构设计](docs/arch/系统内部架构.md)
  - [系统外部架构设计](docs/arch/系统外部架构.md)
  - [各服务间的属性项映射](docs/attr-mapping.md)
  - [云网关测试网络环境](docs/test-network.md)
  - [单点登录流程](docs/arch/单点登录流程.md)
  - [访问控制流程](docs/arch/访问控制流程.md)

- 服务端业务模块
  - [NSAG用户登录访问模式汇总](docs/NSAG用户登录访问模式汇总.md)
  - [TRP代理功能设计](docs/trp/TRP代理功能设计.md)
    - [TRP配置说明](docs/trp/TRP配置说明.md)
    - [TRP信息绑定模块](docs/trp/信息绑定模块设计.md)
    - [TRP反向代理默认配置PPS，SSO](docs/trp/TRP反向代理默认配置PPS，SSO.md)
    - [TRP反向代理认证流程](docs/trp/pps-trp-sso.md)
    - [外部SSO会话自动发现方案](docs/trp/trp_auto_session.md)
      - [会话发现模式-网关与SSO之间不使用traceid来获取身份信息](http://git.koal.com/gw-cloud/gw-cloud-docs/-/issues/820)
    - [TRP-Lua代码仓库说明文档](docs/trp/TRP-Lua脚本仓库说明.md)
    - [TRP禁止文件下载的实现](docs/trp/TRP禁止文件下载的实现.md)
    - [~~TRP基于Redis实现访问控制说明(访问控制鉴权接口统一由Java实现)~~](docs/trp/TRP基于Redis实现访问控制说明.md)
    - [网关性能诊断功能设计](docs/trp/网关性能诊断功能设计.md)
    - [TRP规定SSI接口](docs/trp/TRP提供SSI界面接口.md)
  - [L3VPN与TRP联动设计](docs/L3VPN与TRP联动设计.md)
    - [SSO单点登录服务设计](docs/server-sso-design.md)
    - [SSO注册内部应用](docs/SSO注册内部应用.md)
    - [二次认证](docs/二次认证.md)
    - [SSO相关认证插件配置](docs/SSO相关认证插件配置.md)
    - [用户会话设计](docs/UserSession.md)
    - [SSO服务的管理API](docs/sso_api.md)
    - [二维码扫描登录接口](docs/qrlogin_protocol.md)
  - [PPS策略和门户服务设计](docs/pps/server-pps-design.md)
    - [PPS设计文档](docs/pps/pps设计文档.md)
    - [PPS和客户端交互接口定义](docs/pps/policy/pps-client-api.md)
    - [PPS和客户端交互接口定义(客户端接口)](docs/pps/auth/PPS和客户端交互接口(客户端接口).md)
    - [pps网页和客户端SSO登录设计](docs/pps/auth/pps网页和客户端单点.md)
    - [忘记密码和修改密码设计](docs/pps/auth/用户密码找回和修改.md)
    - [PPS配置详情](docs/pps/pps-config-setting.md)
    - [与4A共享用户信息redis](docs/pps/auth/与4A共享用户信息redis.md)
    - [PPS会话生命周期](docs/pps/auth/PPS会话生命周期.md)
    - [PPS内置SSO设计](docs/pps/auth/PPS内置SSO设计.md)
    - [PPS和客户端认证交互接口](docs/pps/auth/pps和客户端认证交互接口.md)
    - [网关和SSO会话续期功能设](docs/pps/auth/网关和SSO会话续期功能设计.md)
    - [PPS-支持SSL双向认证-设计 ](docs/pps/auth/PPS-支持SSL双向认证-设计.md)
    - [支持外部oauth2协议的SSO-设计](docs/pps/auth/支持外部oauth2协议的SSO-设计.md)
    - [支持网关手机客户端扫码登录-网页](docs/pps/auth/PC客户端扫码登录.md)
    - [测试涉及外网服务配置](docs/pps/auth/测试涉及外网服务配置.md)
    - [Windows程序自动登录-自动填写密码](docs/pps/auth/Windows程序自动登录-自动填写密码.md)
    - [后端事件证书-需求设计](docs/pps/auth/后端事件证书-需求设计.md)
    - [最终用户-通过短信重置口令-需求设计](docs/pps/auth/最终用户-通过短信重置口令-需求设计.md)
    - [用户自己维护开启自动登录的应用的账号密码需求设计](docs/pps/portal/用户自己维护已经开启自动登录的应用的账号密码需求设计.md)
    - [支持外部oauth2协议的SSO需求设计](docs/pps/portal/../auth/支持外部oauth2协议的SSO需求设计.md)
    - [用户自己管理自己的密码信息需求设计](docs/pps/portal/用户自己管理自己的密码信息需求设计.md)
    - [账号锁定功能需求设计](docs/pps/auth/账号锁定功能需求设计.md)
    - [支持多因子认证](docs/pps/auth/支持多因子认证需求设计.md)
    - [会话发现模式-网关与SSO之间不使用traceid来获取身份信息](docs/pps/auth/会话发现模式-网关与SSO之间不使用traceid来获取身份信息需求设计.md)
    - [PPS接入微信认证](docs/pps/auth/PPS接入微信认证需求设计.md)
    - [PPS-优化和改进会话管理](docs/pps/auth/PPS-优化和改进会话管理需求设计.md)
    - [前端事件证书](docs/pps/auth/前端事件证书-需求设计.md)
    - [支持作为证书认证服务器与第三方IAM集成](docs/pps/auth/支持作为证书认证服务器与第三方IAM集成.md)
- 服务端管理模块
  - [RMS用户与权限管理服务设计](docs/server-rms-design.md)
    - [三权管理员设计](docs/三权管理员设计.md)
    - [访问控制权限模型](docs/accesscontrol/访问控制权限模型.md)
      - [Aviator脚本编辑器](docs/accesscontrol/Aviator脚本编辑器.md)
      - [TODO：pe服务双缓冲设计](docs/pe服务双缓冲设计.md)
    - [同步4A资源策略](docs/同步4A资源策略.md)
      - [与4A系统的接口](docs/与4A系统的接口.md)
      - [4A应用同步接口生成TRP的http、tcp正反向代理配置说明](docs/4A应用同步接口生成TRP的http、tcp正反向代理配置说明.md)
    - [每个资源的发布配置单独做成一个json文件](http://git.koal.com/gw-cloud/gw-cloud-docs/-/issues/771#note_144928)
    - [发布反向代理资源](docs/发布反向代理资源.md)
    - [RMS对接SSO设计](docs/rms对接SSO设计.md)
    - [LDAP同步用户和用户组设计](docs/LDAP同步用户和用户组设计.md)
    - [密钥和证书管理服务设计](docs/rms/cks/密钥和证书管理服务设计.md)
    - [TODO：临时用户](docs/临时用户.md)
    - [配置热加载机制](docs/配置热加载机制.md)

  - [系统管理功能设计接口](docs/系统管理功能设计接口.md)
  - [CONFS配置统一管理设计](docs/配置统一管理设计.md)
    - [配置服务使用说明](docs/配置服务使用说明.md)
  - [BDT接口说明](docs/BDT接口说明.md)
  - [LAM日志审计与分析方案](docs/system/log/日志审计与分析方案.md)
    - [审计与监控UI需求](docs/审计与监控UI需求.md)
    - [零信任终端安全评估方案](docs/零信任终端安全评估方案.md)
    - [网关节点状态监控设计](docs/网关节点状态监控设计.md)
    - [网关节点监控同步设计](docs/网关节点监控同步设计.md)
    - [网关审计日志4A对接](http://git.koal.com/gw-cloud/backend/gw-cloud-lam/wikis/%E7%BD%91%E5%85%B3%E5%AE%A1%E8%AE%A1%E6%97%A5%E5%BF%974A%E5%AF%B9%E6%8E%A5.md)
    - [telegraf兼容多输出格式的设计与实现](docs/telegraf兼容多输出格式的设计与实现.md) 
    - [Telegraf定制UserAgent解析](docs/Telegraf定制UserAgent解析.md) 
    - [rsyslog模版](docs/rsyslog模板.md)
    - [重要事件梳理定义](docs/重要事件梳理定义.md)
    - [SYSLOG类别规定](docs/SYSLOG类别规定.md)
  - [DMS](docs/dms/DMS.md)
      - [CPU和内存管理](docs/dms/CPU和内存管理.md)
  - 管理界面
    - [管理界面配置](docs/管理界面配置.md)
- 对外接口设计
    - [NSAG&IGMP对接IAM接口说明](./public-api-docs/NSAG&IGMP对接IAM接口说明.md)
    - [IGMP对接奇安信设备可信状态接口说明](./public-api-docs/IGMP对接奇安信设备可信状态接口说明.md)
    - [IGMP对接格尔审计系统威胁通知接口说明](./public-api-docs/IGMP对接格尔审计系统威胁通知接口说明.md)

- 服务端系统设计
  - 系统部署
    - [安装与卸载](docs/setup/安装与卸载.md)
    - [系统安装指南](docs/setup/系统安装指南.md)
    - [NSAG加入管理平台设计](docs/setup/NSAG加入管理平台设计.md)
    - [平台部署流程设计](docs/平台部署流程设计.md)
  - [看门狗配置](docs/watchdog-howto.md)
  - [License控制设计](docs/License控制设计说明.md)
  - 系统网络
    - [系统内部网络](docs/system/系统内部网络.md)
    - [节点间路由设计](docs/节点间路由设计.md)
    - [Kong路由注册](docs/Kong路由注册.md)
    - [热备和自负载设计](docs/热备和自负载设计.md)
    - [TODO：平台高可用设计](docs/平台高可用设计.md)
  - [服务容器依赖通用检测机制](docs/system/服务容器依赖通用检测机制.md)
  - [云网关配置备份恢复设计](docs/system/云网关配置备份恢复设计.md)
  - [云网关升级设计](docs/system/云网关升级设计.md)

- [客户端架构设计](docs/client-design.md)
  - [客户端与PPS服务认证过程](docs/客户端与PPS服务认证过程.md)
  - [TODO：客户端与服务端接口](docs/客户端与服务端接口.md)
  - [TODO：客户端内部接口](docs/客户端内部接口.md)
- [流程设计](docs/流程设计.md)
- [基于Ticket的认证+SSL代理（旧）](docs/ssl-with-ticket.md)

# [项目方案设计](./project-docs/README.md)

# 测试用例
- [1. 部署](docs/testcase/deploy.md)
- [2. 登录与认证](docs/testcase/auth.md)
- [3. 代理](docs/testcase/proxy.md)
- [4. L3 VPN](docs/testcase/vpn.md)
- [5. 门户](docs/testcase/portal.md)
- [6. 访问控制](docs/testcase/acl.md)
- [7. 客户端相关](docs/testcase/client.md)
- [8. 审计与会话管理](docs/testcase/audit.md)
- [9. 证书与密钥](docs/testcase/cert.md)
- [10. 网络管理](docs/testcase/network.md)
- [11. 系统管理](docs/testcase/system.md)
- [12. BM测评相关](docs/testcase/bm.md)
- [13. 模板](docs/testcase/template.md)
- [14. 审计系统与IGMP集成](docs/testcase/审计系统与IGMP集成测试用例.md)
- [15. IGMP会话管理](docs/testcase/IGMP会话管理.md)
- [16. IGMP策略管理](docs/testcase/IGMP策略管理.md)
- [17. 同步4A策略](docs/testcase/同步4A策略.md)

# 性能测试报告
- [RMS性能测试报告](docs/Performance/RMS性能测试报告.md)
- [PPS性能测试报告](docs/Performance/PPS性能测试报告.md)
- [TRP性能测试报告](docs/Performance/TRP性能测试报告.md)
- [TODO: VPN性能测试报告](docs/Performance/VPN性能测试报告.md)
- [TODO: SSO性能测试报告](docs/Performance/SSO性能测试报告.md)

# 运维指南

- [网关维护指南](docs/网关维护指南.md)
  - [postgresql迁移说明](docs/postgresql迁移说明.md)
# 生产指南

- 见 [生产指南](./factory-docs/README.md)

# 技术预研

- 迁移到 [gw-cloud-skills](http://git.koal.com/gw-cloud/gw-cloud-skills)

# 第三方实现分析
- OpenConnect
  - [OpenConnect客户端](rival-production-docs/OpenConnect客户端代码分析.md)
  - [Openconnect性能测试](rival-production-docs/Openconnect性能测试.md)
  - [openconnect-vpn-协商字段说明](rival-production-docs/openconnect-vpn-协商字段说明.md)
  - [koalVPN、OpenConnect和SangForVPN建立链接速度比对](rival-production-docs/establish_speed.md)
  - [TODO：ocserv服务端](rival-production-docs/ocserv.md)
- 深信服
  - [深信服SSLVPN Windows客户端握手过程](rival-production-docs/sangfor_sslvpn_windows.md)
  - [深信服Android端功能分析](rival-production-docs/深信服Android端功能分析.md)
  - [深信服客户端分析01](rival-production-docs/深信服客户端分析01.md)
  - [深信服客户端分析02-LSP](rival-production-docs/深信服客户端分析02-LSP.md)
- PulseSecure
  - [Connect Secure Dashboard](rival-production-docs/pcs_dashboard)
  - [Connect Secure 终端菜单](rival-production-docs/PulseSecure-Console-Menu.md)
- 信领NAC
  - [三权管理员及BM需求实现](rival-production-docs/xinling_nac_3role.md)
- 信大捷安
  - [客户端同时连接两个VPN](rival-production-docs/xdja_multi_vpn.md)
- Array SSLVPN
  - [Array SSLVPN日志事件列表](rival-production-docs/Array-SSLVPN-LogList.md)
