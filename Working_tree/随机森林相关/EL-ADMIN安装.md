# EL-ADMIN安装（－）安装环境
## Navicat安装
[进入官网](https://www.navicat.com.cn/download/navicat-premium)
选择相应的版本下载
２，授予权限
进入下载文件所在的文件夹执行以下命令

	sudo chmod 777 navicat15-premium-cs.AppImage
3，使用
直接在文件所在的终端执行

	./navicat15-premium-cs.AppImage
### 一些问题
１，连接不上本地Mysql文件
提示没有那个文件或者目录
截图

解决方案：
将localhost
修改为127.0.0.1
2,提示拒绝访问用户
截图

解决方案
修改文件mysqld.cnf

	sudo vim /etc/mysql/mysql.conf.d/mysqld.cnf

在基本设置里面添加`skip-grant-tables`
如图所示

输入下列操作命令作为修改

	use mysql
	
	update mysql.user set authentication_string=password('新密码') where user='root' and Host ='localhost';

	update user set plugin="mysql_native_password";

	flush privileges;

	quit;
	
操作如图所示

重新打开`mysqld.cnf`，删除刚才的`skip-grant-tables`,保存退出。

接下来，通过NAvicat官方网站申请，教育许可永久开通
[网址](https://www.navicat.com.cn/sponsorship/education/student)
填写信息，重点是学校的邮箱。
填写完毕就可以到学校邮箱查看邮件
里面有激活码
填写激活码就可以了。
## 安装IDEA
第一步，官网下载
[官网地址](https://www.jetbrains.com/idea/download/#section=linux)
第二步，申请永久权限（仅限教育相关，就是有学校邮箱的朋友）
[申请地址](https://www.jetbrains.com/zh-cn/community/education/#students)
填写报告，然后到邮箱内接收邮件，然后点击链接绑定账号，ｏｋ
第三步，
解压，安装，登录。