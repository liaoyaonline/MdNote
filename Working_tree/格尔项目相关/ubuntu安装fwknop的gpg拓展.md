# ubuntu安装fwknop的gpg拓展

## 下载依赖

从`gpg`官网下载依赖并进行安装

```
libgpg-error-1.43
libgcrypt-1.9.4
libassuan-2.5.5
libksba-1.6.0
npth-1.6
gnupg-2.3.4
gpa-0.10.0
gpgme-1.16.0
```

源依赖`sm3`

```
sudo apt-get install libssl-dev
```

流程大致如下

安装完依赖，重新编译，确定`./configure`时，显示`gpg`拓展为`yes`

在服务端生成密钥，修改配置文件，生成主密钥，然后将生成子密钥。将密钥导出放到客户端。在客户端将该密钥导入密钥环中，并使用客户端的主密钥对导入的服务端的密钥进行签名。

在服务端生成主密钥，修改配置文件，生成子密钥，将密钥导出放到服务端，在服务端将该密钥导入到密钥环中，并使用服务端的主密钥对导入的客户端的密钥进行签名。

在`/root/.gngpg/gpg.conf`中增加下列字符串，如果没有gpg.conf则新增该文件

```
keyid-format long
with-fingerprint
```

在服务端使用新增密钥，注意使用`root`用户，因为服务端是需要在`root`权限下运行的，它需要修改防火墙规则。所以它检测的时候会检测`/root/.gngpg/`目录下保存的密钥。

```
root@ubuntu:/home/liaoya/github/test/testgpg# gpg --expert --full-gen-key
gpg (GnuPG) 2.3.4; Copyright (C) 2021 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

请选择您要使用的密钥类型：
   (1) RSA 和 RSA 
   (2) DSA 和 Elgamal 
   (3) DSA（仅用于签名）
   (4) RSA（仅用于签名）
   (7) DSA（自定义用途）
   (8) RSA（自定义用途）
   (9) ECC（签名和加密） *默认*
  (10) ECC（仅用于签名）
  (11) ECC（自定义用途）
  (13) 现有密钥 
 （14）卡中现有密钥 
您的选择是？ 8
                    
RSA 密钥的可实现的功能： 签名（Sign） 认证（Certify） 加密（Encrypt） 身份验证（Authenticate） 
目前启用的功能： 签名（Sign） 认证（Certify） 加密（Encrypt） 

   (S) 签名功能开关
   (E) 加密功能开关
   (A) 身份验证功能开关
   (Q) 已完成

您的选择是？ S
                    
RSA 密钥的可实现的功能： 签名（Sign） 认证（Certify） 加密（Encrypt） 身份验证（Authenticate） 
目前启用的功能： 认证（Certify） 加密（Encrypt） 

   (S) 签名功能开关
   (E) 加密功能开关
   (A) 身份验证功能开关
   (Q) 已完成

您的选择是？ E
                    
RSA 密钥的可实现的功能： 签名（Sign） 认证（Certify） 加密（Encrypt） 身份验证（Authenticate） 
目前启用的功能： 认证（Certify） 

   (S) 签名功能开关
   (E) 加密功能开关
   (A) 身份验证功能开关
   (Q) 已完成

您的选择是？ Q
RSA 密钥的长度应在 1024 位与 4096 位之间。
您想要使用的密钥长度？(3072) 1024
请求的密钥长度是 1024 位                    
请设定这个密钥的有效期限。
         0 = 密钥永不过期
      <n>  = 密钥在 n 天后过期
      <n>w = 密钥在 n 周后过期
      <n>m = 密钥在 n 月后过期
      <n>y = 密钥在 n 年后过期
密钥的有效期限是？(0) 1y
密钥于 Sat 11 Feb 2023 03:48:28 PM CST 过期
这些内容正确吗？ (y/N) y
                                
GnuPG 需要构建用户标识以辨认您的密钥。

真实姓名： test
姓名至少要有五个字符长
真实姓名： testfwknop
电子邮件地址： testfwknopd@test.com
注释：                                    
您选定了此用户标识：
    “testfwknop <testfwknopd@test.com>”

更改姓名（N）、注释（C）、电子邮件地址（E）或确定（O）/退出（Q）？ O
                                                                                我们需要生成大量的随机字节。在质数生成期间做些其他操作（敲打键盘
、移动鼠标、读写硬盘之类的）将会是一个不错的主意；这会让随机数
发生器有更好的机会获得足够的熵。
gpg: 警告：服务器 ‘gpg-agent’ 比我们的版本更老 （2.2.4 < 2.3.4）
gpg: 注意： 过时的服务器可能缺少重要的安全修复。
gpg: 注意： 使用 “gpgconf --kill all” 来重启他们。
gpg: 吊销证书已被存储为‘/root/.gnupg/openpgp-revocs.d/1651479B107806ED57A227A70B3B0B0E32637599.rev’
公钥和私钥已经生成并被签名。

pub   rsa1024/0B3B0B0E32637599 2022-02-11 [C] [有效至：2023-02-11]
      密钥指纹 = 1651 479B 1078 06ED 57A2  27A7 0B3B 0B0E 3263 7599
uid                            testfwknop <testfwknopd@test.com>
```

然后在其下生成子秘钥

```
root@ubuntu:/home/liaoya/github/test/testgpg# gpg --expert --edit-key 0B3B0B0E32637599
gpg (GnuPG) 2.3.4; Copyright (C) 2021 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

gpg: 警告：服务器 ‘gpg-agent’ 比我们的版本更老 （2.2.4 < 2.3.4）
gpg: 注意： 过时的服务器可能缺少重要的安全修复。
gpg: 注意： 使用 “gpgconf --kill all” 来重启他们。
gpg: problem with fast path key listing: IPC 参数错误 - ignored
私钥可用。

gpg: 正在检查信任度数据库
gpg: marginals needed: 3  completes needed: 1  trust model: pgp
gpg: 深度：0  有效性：  2  已签名：  1  信任度：0-，0q，0n，0m，0f，2u
gpg: 深度：1  有效性：  1  已签名：  0  信任度：1-，0q，0n，0m，0f，0u
gpg: 下次信任度数据库检查将于 2023-02-11 进行
sec  rsa1024/0B3B0B0E32637599
     创建于：2022-02-11  有效至：2023-02-11  可用于：C   
     信任度：绝对        有效性：绝对
[ 绝对 ] (1). testfwknop <testfwknopd@test.com>

gpg> addkey
请选择您要使用的密钥类型：
   (3) DSA（仅用于签名）
   (4) RSA（仅用于签名）
   (5) ElGamal（仅用于加密）
   (6) RSA（仅用于加密）
   (7) DSA（自定义用途）
   (8) RSA（自定义用途）
  (10) ECC（仅用于签名）
  (11) ECC（自定义用途）
  (12) ECC（仅用于加密）
  (13) 现有密钥 
 （14）卡中现有密钥 
您的选择是？ 4
RSA 密钥的长度应在 1024 位与 4096 位之间。
您想要使用的密钥长度？(3072) 2048
请求的密钥长度是 2048 位                    
请设定这个密钥的有效期限。
         0 = 密钥永不过期
      <n>  = 密钥在 n 天后过期
      <n>w = 密钥在 n 周后过期
      <n>m = 密钥在 n 月后过期
      <n>y = 密钥在 n 年后过期
密钥的有效期限是？(0) 1y
密钥于 Sat 11 Feb 2023 03:54:34 PM CST 过期
这些内容正确吗？ (y/N) y
真的要创建吗？(y/N) y           
我们需要生成大量的随机字节。在质数生成期间做些其他操作（敲打键盘
、移动鼠标、读写硬盘之类的）将会是一个不错的主意；这会让随机数
发生器有更好的机会获得足够的熵。

sec  rsa1024/0B3B0B0E32637599
     创建于：2022-02-11  有效至：2023-02-11  可用于：C   
     信任度：绝对        有效性：绝对
ssb  rsa2048/7C5E84E1FD3D8CD3
     创建于：2022-02-11  有效至：2023-02-11  可用于：S   
[ 绝对 ] (1). testfwknop <testfwknopd@test.com>

gpg> save
```

以此类推，生成一个只用于加密的子秘钥，生成一个只用于认证的子秘钥。

导出服务端的密钥

```
root@ubuntu:/home/liaoya/github/test/testgpg# gpg -a --export 0B3B0B0E32637599 > server.asc
```

以此类推，在客户端生成主密钥，子秘钥，然后导出来。将客户端导出的密钥移动到服务端，将服务端导出的密钥移动到客户端，然后将客户端的密钥导入服务端的密钥环。将服务端的密钥导入客户端的密钥环。

将客户端的密钥导入服务端的密钥环

```
root@ubuntu:/home/liaoya/github/test/testgpg# gpg --import client.asc 
gpg: 密钥 D5FA0C312EA8FED2：公钥 “fwknop-client <fwknop-client@koal.com>” 已导入
gpg: 警告：服务器 ‘gpg-agent’ 比我们的版本更老 （2.2.4 < 2.3.4）
gpg: 注意： 过时的服务器可能缺少重要的安全修复。
gpg: 注意： 使用 “gpgconf --kill all” 来重启他们。
gpg: 处理的总数：1
gpg:               已导入：1
root@ubuntu:/home/liaoya/github/test/testgpg# gpg -k
/root/.gnupg/pubring.kbx
------------------------
pub   rsa1024/A589375B13ECDCF3 2022-02-11 [C] [有效至：2023-02-11]
      密钥指纹 = 78BF CCCA 11DB 0E63 BD40  BC86 A589 375B 13EC DCF3
uid                   [ 绝对 ] fwknopd-server <fwknopd-server@koal.com>
sub   rsa2048/C28F899563BD2DDB 2022-02-11 [S] [有效至：2023-02-11]
sub   rsa2048/479E2D976400E003 2022-02-11 [E] [有效至：2023-02-11]
sub   rsa2048/4FA0D36E5E844449 2022-02-11 [A] [有效至：2023-02-11]

pub   rsa1024/D5FA0C312EA8FED2 2022-02-11 [C] [有效至：2023-02-11]
      密钥指纹 = E7D7 062A 9869 243D 5BF0  BD73 D5FA 0C31 2EA8 FED2
uid                   [ 未知 ] fwknop-client <fwknop-client@koal.com>
sub   rsa2048/326497139AE8CFA1 2022-02-11 [S] [有效至：2023-02-11]
sub   rsa2048/FEE14C3D09CCEA57 2022-02-11 [E] [有效至：2023-02-11]
sub   rsa2048/9D7996FA0C8B53DB 2022-02-11 [A] [有效至：2023-02-11]
```

对导入的客户端密钥进行签名

```
root@ubuntu:/home/liaoya/github/test/testgpg# gpg --edit-key D5FA0C312EA8FED2
gpg (GnuPG) 2.3.4; Copyright (C) 2021 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

gpg: 警告：服务器 ‘gpg-agent’ 比我们的版本更老 （2.2.4 < 2.3.4）
gpg: 注意： 过时的服务器可能缺少重要的安全修复。
gpg: 注意： 使用 “gpgconf --kill all” 来重启他们。
gpg: problem with fast path key listing: IPC 参数错误 - ignored

pub  rsa1024/D5FA0C312EA8FED2
     创建于：2022-02-11  有效至：2023-02-11  可用于：C   
     信任度：未知        有效性：未知
sub  rsa2048/326497139AE8CFA1
     创建于：2022-02-11  有效至：2023-02-11  可用于：S   
sub  rsa2048/FEE14C3D09CCEA57
     创建于：2022-02-11  有效至：2023-02-11  可用于：E   
sub  rsa2048/9D7996FA0C8B53DB
     创建于：2022-02-11  有效至：2023-02-11  可用于：A   
[ 未知 ] (1). fwknop-client <fwknop-client@koal.com>

gpg> sign
         
pub  rsa1024/D5FA0C312EA8FED2
     创建于：2022-02-11  有效至：2023-02-11  可用于：C   
     信任度：未知        有效性：未知
 主密钥指纹： E7D7 062A 9869 243D 5BF0  BD73 D5FA 0C31 2EA8 FED2

     fwknop-client <fwknop-client@koal.com>

这个密钥将在 2023-02-11 过期。
您真的确定要签名这个密钥，使用您的密钥
“fwknopd-server <fwknopd-server@koal.com>” (A589375B13ECDCF3)

真的要签名吗？(y/N) y
                            
gpg> save
root@ubuntu:/home/liaoya/github/test/testgpg# gpg -k
gpg: 正在检查信任度数据库
gpg: marginals needed: 3  completes needed: 1  trust model: pgp
gpg: 深度：0  有效性：  1  已签名：  1  信任度：0-，0q，0n，0m，0f，1u
gpg: 深度：1  有效性：  1  已签名：  0  信任度：1-，0q，0n，0m，0f，0u
gpg: 下次信任度数据库检查将于 2023-02-11 进行
/root/.gnupg/pubring.kbx
------------------------
pub   rsa1024/A589375B13ECDCF3 2022-02-11 [C] [有效至：2023-02-11]
      密钥指纹 = 78BF CCCA 11DB 0E63 BD40  BC86 A589 375B 13EC DCF3
uid                   [ 绝对 ] fwknopd-server <fwknopd-server@koal.com>
sub   rsa2048/C28F899563BD2DDB 2022-02-11 [S] [有效至：2023-02-11]
sub   rsa2048/479E2D976400E003 2022-02-11 [E] [有效至：2023-02-11]
sub   rsa2048/4FA0D36E5E844449 2022-02-11 [A] [有效至：2023-02-11]

pub   rsa1024/D5FA0C312EA8FED2 2022-02-11 [C] [有效至：2023-02-11]
      密钥指纹 = E7D7 062A 9869 243D 5BF0  BD73 D5FA 0C31 2EA8 FED2
uid                   [ 完全 ] fwknop-client <fwknop-client@koal.com>
sub   rsa2048/326497139AE8CFA1 2022-02-11 [S] [有效至：2023-02-11]
sub   rsa2048/FEE14C3D09CCEA57 2022-02-11 [E] [有效至：2023-02-11]
sub   rsa2048/9D7996FA0C8B53DB 2022-02-11 [A] [有效至：2023-02-11]
```

对于多余的密钥，可以将其删除

```
root@ubuntu:/home/liaoya/github/fwknop-sm3/fwknop-2.6.10# gpg --delete-secret-and-public-key 0B3B0B0E32637599
gpg (GnuPG) 2.3.4; Copyright (C) 2021 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

gpg: 警告：服务器 ‘gpg-agent’ 比我们的版本更老 （2.2.4 < 2.3.4）
gpg: 注意： 过时的服务器可能缺少重要的安全修复。
gpg: 注意： 使用 “gpgconf --kill all” 来重启他们。

sec  rsa1024/0B3B0B0E32637599 2022-02-11 testfwknop <testfwknopd@test.com>

要从钥匙环里删除这个密钥吗？(y/N) y
这是一个私钥！- 真的要删除吗？(y/N) y            
                                                   
pub  rsa1024/0B3B0B0E32637599 2022-02-11 testfwknop <testfwknopd@test.com>

要从钥匙环里删除这个密钥吗？(y/N) y
root@ubuntu:/home/liaoya/github/fwknop-sm3/fwknop-2.6.10# gpg -k
gpg: 正在检查信任度数据库
gpg: marginals needed: 3  completes needed: 1  trust model: pgp
gpg: 深度：0  有效性：  1  已签名：  1  信任度：0-，0q，0n，0m，0f，1u
gpg: 深度：1  有效性：  1  已签名：  0  信任度：1-，0q，0n，0m，0f，0u
gpg: 下次信任度数据库检查将于 2023-02-11 进行
/root/.gnupg/pubring.kbx
------------------------
pub   rsa1024/A589375B13ECDCF3 2022-02-11 [C] [有效至：2023-02-11]
      密钥指纹 = 78BF CCCA 11DB 0E63 BD40  BC86 A589 375B 13EC DCF3
uid                   [ 绝对 ] fwknopd-server <fwknopd-server@koal.com>
sub   rsa2048/C28F899563BD2DDB 2022-02-11 [S] [有效至：2023-02-11]
sub   rsa2048/479E2D976400E003 2022-02-11 [E] [有效至：2023-02-11]
sub   rsa2048/4FA0D36E5E844449 2022-02-11 [A] [有效至：2023-02-11]

pub   rsa1024/D5FA0C312EA8FED2 2022-02-11 [C] [有效至：2023-02-11]
      密钥指纹 = E7D7 062A 9869 243D 5BF0  BD73 D5FA 0C31 2EA8 FED2
uid                   [ 完全 ] fwknop-client <fwknop-client@koal.com>
sub   rsa2048/326497139AE8CFA1 2022-02-11 [S] [有效至：2023-02-11]
sub   rsa2048/FEE14C3D09CCEA57 2022-02-11 [E] [有效至：2023-02-11]
sub   rsa2048/9D7996FA0C8B53DB 2022-02-11 [A] [有效至：2023-02-11]
```

在服务端修改配置文件`/etc/fwknop/access.conf`

```
### for 10.0.40.74
SOURCE              ANY
OPEN_PORTS              tcp/22
DATA_COLLECT_MODE       PCAP
REQUIRE_SOURCE_ADDRESS  Y
#HMAC_DIGEST_TYPE       sm3
GPG_REMOTE_ID          D5FA0C312EA8FED2
GPG_DECRYPT_ID         A589375B13ECDCF3
GPG_DECRYPT_PW         123
#KEY_BASE64     rB5AymQukIQIbaQQkW8jwVjR2hnrBQc6qGz9Y07WvpY=
#HMAC_KEY_BASE64        btFsqMYO0wHA9AFN1Lwrv2DCXu+dBVXgDML9mVRXig7cVmkm1SVidJUoQ9xzNfqE06KABpalx6woB2GSL85m4g==
GPG_HOME_DIR           /root/.gnupg
FW_ACCESS_TIMEOUT          60
```



在客户端修改配置文件

```
fwknop -A tcp/22 --gpg-recip A58xxxx --gpg-sign Dxxxxx -a 10.0.40.74 -D 10.0.40.72 --verbose -R
```

**-A, --access** = *<端口列表>*

- 提供要在运行 **fwknopd**的远程计算机上访问的端口和协议列表。该列表的格式为“+<proto>/<port>...<proto>/<port>+”，例如“tcp/22,udp/53”。 **注意：**fwknop的绝大多数用法 都需要 **-A** 参数，但通过 SPA 数据包发送带有 **--server-cmd** 参数的完整命令以由 **fwknopd** 执行不需要此参数。

**--gpg-recipient** = *<密钥 ID 或名称>*

- 指定 GnuPG 密钥 ID，例如“+1234ABCD+”（参见“gpg---list-keys”的输出）或单包授权消息接收者的密钥名称（关联的电子邮件地址）。此密钥由 **fwknopd** 服务器导入，关联的私钥用于解密 SPA 数据包。收件人的密钥必须首先导入客户端 GnuPG 密钥环。

**--gpg-signer-key** = *<密钥 ID 或名称>*

- 指定 GnuPG 密钥 ID，例如“+ABCD1234+”（参见“gpg --list-keys”的输出）或签名 SPA 消息时使用的密钥名称。系统会提示用户输入关联的 GnuPG 密码以创建签名。这增加了一种强大的加密机制，允许远程服务器上的 **fwknopd** 守护进程对创建 SPA 消息的人进行身份验证。

**-a, --allow-ip** = *<IP地址>*

- 指定应允许通过目标 **fwknopd** 服务器防火墙的 IP 地址（此 IP 在 SPA 数据包本身内加密）。这对于防止 SPA 数据包可以在途中被截获并从与原始 IP 不同的 IP 发送的 MITM 攻击很有用。因此，如果 **fwknopd** 服务器信任 SPA 数据包 IP 标头上的源地址，那么攻击者将获得访问权限。**-a**选项将 源地址放在加密的 SPA 数据包中，因此可以阻止这种攻击。**-a** 选项还可用于指定当 SPA 数据包本身被 --spoof-src 选项欺骗时将被授予访问权限 的 **IP** 。另一个相关选项是 **-R** （见下文）指示 **fwknop客户端通过查询***https://www.cipherdyne.org/cgi-bin/myip* 自动解析本地系统连接到的外部可路由 IP 地址 。这将返回它从调用系统看到的实际 IP 地址。

**-D, --destination** = *<主机名/IP地址>*

- **指示 fwknop 客户端使用指定目标主机名或 IP 地址的 fwknopd** 守护 程序 /服务**进行身份验证。****fwknopd** 守护进程/服务在解密和解析身份验证数据包时 发现连接模式 。

**-v, --verbose**

- 以详细模式 运行 fwknop客户端。这会导致 fwknop 打印有关当前命令和生成的 SPA 数据的一些额外信息。

**-R, --resolve-ip-https**

- 这是一个重要的选项，它指示 **fwknop客户端向***cipherdyne.org* 上运行的脚本发出 HTTPS 请求，该脚本 返回客户端的 IP 地址（如 Web 服务器所见）。在某些情况下，这需要确定应允许通过远程 **fwknopd** 服务器端的防火墙策略的 IP 地址。**如果fwknop** 客户端在隐藏的 NAT 地址后面的系统上使用，并且用户不知道面向外部 Internet 的 IP，则此选项很有用 。完整解析 URL 为： *https://www.cipherdyne.org/cgi-bin/myip*，由 **fwknop** 通过 *wget* in **--secure-protocol访问** 模式。**请注意，如果用户已经知道客户端的外部可路由 IP 地址，则使用-a**选项通常更安全， 因为这消除了 **fwknop** 发出任何类型的 HTTPS 请求的需要。



检查服务器端口开放情况


```
sudo nmap -sS -p 22 10.0.40.72
```

