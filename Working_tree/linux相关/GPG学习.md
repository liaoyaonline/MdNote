# GPG学习

## 常用命令

```
gpg --homedir
仅在命令行有效，指定生成的密钥环以及其他数据保存的地方
```

|                          | 简写 | 含义                         |
| ------------------------ | ---- | ---------------------------- |
| key pair                 |      | 密钥对(包含两部分)           |
| primary key [master key] |      | 主密钥（包括主公钥，主私钥） |
| public key               | pub  | 公钥                         |
| secret key               | sec  | 私钥                         |
| sub-key                  | sub  | 子公钥                       |
| secret sub-key           | ssb  | 子私钥                       |
| key fingerprint          |      | 密钥指纹                     |

#### gpg --list-keys

#### gpg -k

列出公钥，如果没有指定则列出所有

#### gpg --list-secret-keys

#### gpg -K

列出私钥，如果没有指定则列出所有

**--generate-revocation**

创建吊销证书

```
gpg --generate-revocation keyId --out path/revoke.asc
```

```
# 创建配置文件
gpg.conf

# 添加如下内容
################################################################################
# GnuPG View Options

# 显示keyId 16位 16进制数字
# Select how to display key IDs. "long" is the more accurate (but less 
# convenient) 16-character key ID. Add an "0x" to include an "0x" at the 
# beginning of the key ID. 
keyid-format long

# 显示指纹标识
# List all keys with their fingerprints. This is the same output as --list-keys 
# but with the additional output of a line with the fingerprint. If this 
# command is given twice, the fingerprints of all secondary keys are listed too.
with-fingerprint

# 保存退出
```

### 生成主密钥

```bash
lfp@legion:~$ mkdir ~/.gnupg-test
lfp@legion:~$ vim ~/.gnupg-test/gpg.conf
lfp@legion:~$ gpg --homedir ~/.gnupg-test --expert --full-gen-key
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg (GnuPG) 2.2.4; Copyright (C) 2017 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

gpg: keybox '/home/lfp/.gnupg-test/pubring.kbx' created
请选择您要使用的密钥种类：
   (1) RSA and RSA (default)
   (2) DSA and Elgamal
   (3) DSA (仅用于签名)
   (4) RSA (仅用于签名)
   (7) DSA (自定义用途)
   (8) RSA (自定义用途)
   (9) ECC and ECC
  (10) ECC (sign only)
  (11) ECC (set your own capabilities)
  (13) Existing key
您的选择？ 8

RSA 密钥可能的操作： 签名 验证 加密 认证 
目前允许的操作： 签名 验证 加密 

   (S) 选择是否用于签名
   (E) 选择是否用于加密
   (A) 选择是否用于认证
   (Q) 已完成

您的选择？ s

RSA 密钥可能的操作： 签名 验证 加密 认证 
目前允许的操作： 验证 加密 

   (S) 选择是否用于签名
   (E) 选择是否用于加密
   (A) 选择是否用于认证
   (Q) 已完成

您的选择？ e

RSA 密钥可能的操作： 签名 验证 加密 认证 
目前允许的操作： 验证 

   (S) 选择是否用于签名
   (E) 选择是否用于加密
   (A) 选择是否用于认证
   (Q) 已完成

您的选择？ q
RSA 密钥长度应在 1024 位与 4096 位之间。
您想要用多大的密钥尺寸？(3072) 4096
您所要求的密钥尺寸是 4096 位
请设定这把密钥的有效期限。
         0 = 密钥永不过期
      <n>  = 密钥在 n 天后过期
      <n>w = 密钥在 n 周后过期
      <n>m = 密钥在 n 月后过期
      <n>y = 密钥在 n 年后过期
密钥的有效期限是？(0) 1
密钥于 2020年05月04日 星期一 19时53分23秒 CST 过期
以上正确吗？(y/n) y

You need a user ID to identify your key; the software constructs the user ID
from the Real Name, Comment and Email Address in this form:
    "Heinrich Heine (Der Dichter) <heinrichh@duesseldorf.de>"

真实姓名： usmile
电子邮件地址： usmile@qq.com
注释： 
您选定了这个用户标识：
    “usmile <usmile@qq.com>”

更改姓名(N)、注释(C)、电子邮件地址(E)或确定(O)/退出(Q)？ o
# 此时需要输入一个口令，一定要记住，后面编辑该密钥时需要用到
我们需要生成大量的随机字节。这个时候您可以多做些琐事(像是敲打键盘、移动
鼠标、读写硬盘之类的)，这会让随机数字发生器有更好的机会获得足够的熵数。
gpg: /home/lfp/.gnupg-test/trustdb.gpg：建立了信任度数据库
gpg: 密钥 0x2954C91DEAE3C032 被标记为绝对信任
gpg: directory '/home/lfp/.gnupg-test/openpgp-revocs.d' created
gpg: revocation certificate stored as '/home/lfp/.gnupg-test/openpgp-revocs.d/76D139A8F9172972195CF33C2954C91DEAE3C032.rev'
公钥和私钥已经生成并经签名。

pub   rsa4096/0x2954C91DEAE3C032 2020-05-03 [C] [有效至：2020-05-04]
      密钥指纹 = 76D1 39A8 F917 2972 195C  F33C 2954 C91D EAE3 C032
uid                              usmile <usmile@qq.com>
```

#### 检查是否成功

```bash
lfp@legion:~$ gpg --homedir ~/.gnupg-test -K
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg: 正在检查信任度数据库
gpg: marginals needed: 3  completes needed: 1  trust model: pgp
gpg: 深度：0 有效性：  1 已签名：  0 信任度：0-，0q，0n，0m，0f，1u
gpg: 下次信任度数据库检查将于 2020-05-04 进行
/home/lfp/.gnupg-test/pubring.kbx
---------------------------------
sec   rsa4096/0x2954C91DEAE3C032 2020-05-03 [C] [有效至：2020-05-04]
      密钥指纹 = 76D1 39A8 F917 2972 195C  F33C 2954 C91D EAE3 C032
uid                   [ 绝对 ] usmile <usmile@qq.com>
```

### 创建子密钥[S]

```bash
lfp@legion:~$ gpg --homedir ~/.gnupg-test --expert --edit-key 0x2954C91DEAE3C032 
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg (GnuPG) 2.2.4; Copyright (C) 2017 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

私钥可用。

sec  rsa4096/0x2954C91DEAE3C032
     创建于：2020-05-03  有效至：2020-05-04  可用于：C   
     信任度：绝对        有效性：绝对
[ 绝对 ] (1). usmile <usmile@qq.com>

gpg> addkey
请选择您要使用的密钥种类：
   (3) DSA (仅用于签名)
   (4) RSA (仅用于签名)
   (5) ElGamal (仅用于加密)
   (6) RSA (仅用于加密)
   (7) DSA (自定义用途)
   (8) RSA (自定义用途)
  (10) ECC (sign only)
  (11) ECC (set your own capabilities)
  (12) ECC (encrypt only)
  (13) Existing key
您的选择？ 4
RSA 密钥长度应在 1024 位与 4096 位之间。
您想要用多大的密钥尺寸？(3072) 
您所要求的密钥尺寸是 3072 位
请设定这把密钥的有效期限。
         0 = 密钥永不过期
      <n>  = 密钥在 n 天后过期
      <n>w = 密钥在 n 周后过期
      <n>m = 密钥在 n 月后过期
      <n>y = 密钥在 n 年后过期
密钥的有效期限是？(0) 1
密钥于 2020年05月04日 星期一 19时57分58秒 CST 过期
以上正确吗？(y/n) y
真的要建立吗？(y/N) y
我们需要生成大量的随机字节。这个时候您可以多做些琐事(像是敲打键盘、移动
鼠标、读写硬盘之类的)，这会让随机数字发生器有更好的机会获得足够的熵数。

sec  rsa4096/0x2954C91DEAE3C032
     创建于：2020-05-03  有效至：2020-05-04  可用于：C   
     信任度：绝对        有效性：绝对
ssb  rsa3072/0x6E3823DE57F89766
     创建于：2020-05-03  有效至：2020-05-04  可用于：S   
[ 绝对 ] (1). usmile <usmile@qq.com>

# 注意一定要保存才能生效
gpg> save
```

#### 查看是否创建成功

```bash
lfp@legion:~$ gpg --homedir ~/.gnupg-test -K
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
/home/lfp/.gnupg-test/pubring.kbx
---------------------------------
sec   rsa4096/0x2954C91DEAE3C032 2020-05-03 [C] [有效至：2020-05-04]
      密钥指纹 = 76D1 39A8 F917 2972 195C  F33C 2954 C91D EAE3 C032
uid                   [ 绝对 ] usmile <usmile@qq.com>
ssb   rsa3072/0x6E3823DE57F89766 2020-05-03 [S] [有效至：2020-05-04]

lfp@legion:~$ gpg --homedir ~/.gnupg-test -k
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
/home/lfp/.gnupg-test/pubring.kbx
---------------------------------
pub   rsa4096/0x2954C91DEAE3C032 2020-05-03 [C] [有效至：2020-05-04]
      密钥指纹 = 76D1 39A8 F917 2972 195C  F33C 2954 C91D EAE3 C032
uid                   [ 绝对 ] usmile <usmile@qq.com>
sub   rsa3072/0x6E3823DE57F89766 2020-05-03 [S] [有效至：2020-05-04]
```

### 创建子密钥[A]

```bash
lfp@legion:~$ gpg --homedir ~/.gnupg-test --expert --edit-key 0x2954C91DEAE3C032 
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg (GnuPG) 2.2.4; Copyright (C) 2017 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

私钥可用。

sec  rsa4096/0x2954C91DEAE3C032
     创建于：2020-05-03  有效至：2020-05-04  可用于：C   
     信任度：绝对        有效性：绝对
ssb  rsa3072/0x6E3823DE57F89766
     创建于：2020-05-03  有效至：2020-05-04  可用于：S   
[ 绝对 ] (1). usmile <usmile@qq.com>

gpg> addkey
请选择您要使用的密钥种类：
   (3) DSA (仅用于签名)
   (4) RSA (仅用于签名)
   (5) ElGamal (仅用于加密)
   (6) RSA (仅用于加密)
   (7) DSA (自定义用途)
   (8) RSA (自定义用途)
  (10) ECC (sign only)
  (11) ECC (set your own capabilities)
  (12) ECC (encrypt only)
  (13) Existing key
您的选择？ 8

RSA 密钥可能的操作： 签名 加密 认证 
目前允许的操作： 签名 加密 

   (S) 选择是否用于签名
   (E) 选择是否用于加密
   (A) 选择是否用于认证
   (Q) 已完成

您的选择？ s

RSA 密钥可能的操作： 签名 加密 认证 
目前允许的操作： 加密 

   (S) 选择是否用于签名
   (E) 选择是否用于加密
   (A) 选择是否用于认证
   (Q) 已完成

您的选择？ e

RSA 密钥可能的操作： 签名 加密 认证 
目前允许的操作： 

   (S) 选择是否用于签名
   (E) 选择是否用于加密
   (A) 选择是否用于认证
   (Q) 已完成

您的选择？ a

RSA 密钥可能的操作： 签名 加密 认证 
目前允许的操作： 认证 

   (S) 选择是否用于签名
   (E) 选择是否用于加密
   (A) 选择是否用于认证
   (Q) 已完成

您的选择？ q
RSA 密钥长度应在 1024 位与 4096 位之间。
您想要用多大的密钥尺寸？(3072) 
您所要求的密钥尺寸是 3072 位
请设定这把密钥的有效期限。
         0 = 密钥永不过期
      <n>  = 密钥在 n 天后过期
      <n>w = 密钥在 n 周后过期
      <n>m = 密钥在 n 月后过期
      <n>y = 密钥在 n 年后过期
密钥的有效期限是？(0) 1
密钥于 2020年05月04日 星期一 19时59分46秒 CST 过期
以上正确吗？(y/n) y
真的要建立吗？(y/N) y
我们需要生成大量的随机字节。这个时候您可以多做些琐事(像是敲打键盘、移动
鼠标、读写硬盘之类的)，这会让随机数字发生器有更好的机会获得足够的熵数。

sec  rsa4096/0x2954C91DEAE3C032
     创建于：2020-05-03  有效至：2020-05-04  可用于：C   
     信任度：绝对        有效性：绝对
ssb  rsa3072/0x6E3823DE57F89766
     创建于：2020-05-03  有效至：2020-05-04  可用于：S   
ssb  rsa3072/0x2CA24BFA9B038328
     创建于：2020-05-03  有效至：2020-05-04  可用于：A   
[ 绝对 ] (1). usmile <usmile@qq.com>

gpg> save
```

#### 查看是否创建成功

```bash
lfp@legion:~$ gpg --homedir ~/.gnupg-test -K
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
/home/lfp/.gnupg-test/pubring.kbx
---------------------------------
sec   rsa4096/0x2954C91DEAE3C032 2020-05-03 [C] [有效至：2020-05-04]
      密钥指纹 = 76D1 39A8 F917 2972 195C  F33C 2954 C91D EAE3 C032
uid                   [ 绝对 ] usmile <usmile@qq.com>
ssb   rsa3072/0x6E3823DE57F89766 2020-05-03 [S] [有效至：2020-05-04]
ssb   rsa3072/0x2CA24BFA9B038328 2020-05-03 [A] [有效至：2020-05-04]
```

### 添加uid

```sql
lfp@legion:~$ gpg --homedir ~/.gnupg-test --expert --edit-key 0x2954C91DEAE3C032
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg (GnuPG) 2.2.4; Copyright (C) 2017 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

私钥可用。

sec  rsa4096/0x2954C91DEAE3C032
     创建于：2020-05-03  有效至：2020-05-04  可用于：C   
     信任度：绝对        有效性：绝对
ssb  rsa3072/0x6E3823DE57F89766
     创建于：2020-05-03  有效至：2020-05-04  可用于：S   
ssb  rsa3072/0x2CA24BFA9B038328
     创建于：2020-05-03  有效至：2020-05-04  可用于：A   
[ 绝对 ] (1). usmile <usmile@qq.com>

gpg> adduid
真实姓名： usmile
电子邮件地址： usmile@wx.com
注释： 
您选定了这个用户标识：
    “usmile <usmile@wx.com>”

更改姓名(N)、注释(C)、电子邮件地址(E)或确定(O)/退出(Q)？ o

sec  rsa4096/0x2954C91DEAE3C032
     创建于：2020-05-03  有效至：2020-05-04  可用于：C   
     信任度：绝对        有效性：绝对
ssb  rsa3072/0x6E3823DE57F89766
     创建于：2020-05-03  有效至：2020-05-04  可用于：S   
ssb  rsa3072/0x2CA24BFA9B038328
     创建于：2020-05-03  有效至：2020-05-04  可用于：A   
[ 绝对 ] (1)  usmile <usmile@qq.com>
[ 未知 ] (2). usmile <usmile@wx.com>

# 选择上方 [未知] uid 的编号
gpg> uid 2

sec  rsa4096/0x2954C91DEAE3C032
     创建于：2020-05-03  有效至：2020-05-04  可用于：C   
     信任度：绝对        有效性：绝对
ssb  rsa3072/0x6E3823DE57F89766
     创建于：2020-05-03  有效至：2020-05-04  可用于：S   
ssb  rsa3072/0x2CA24BFA9B038328
     创建于：2020-05-03  有效至：2020-05-04  可用于：A   
[ 绝对 ] (1)  usmile <usmile@qq.com>
[ 未知 ] (2)* usmile <usmile@wx.com>

gpg> primary

sec  rsa4096/0x2954C91DEAE3C032
     创建于：2020-05-03  有效至：2020-05-04  可用于：C   
     信任度：绝对        有效性：绝对
ssb  rsa3072/0x6E3823DE57F89766
     创建于：2020-05-03  有效至：2020-05-04  可用于：S   
ssb  rsa3072/0x2CA24BFA9B038328
     创建于：2020-05-03  有效至：2020-05-04  可用于：A   
[ 绝对 ] (1)  usmile <usmile@qq.com>
[ 未知 ] (2)* usmile <usmile@wx.com>

gpg> save
```

#### 查看是否添加成功

```bash
lfp@legion:~$ gpg --homedir ~/.gnupg-test -K
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
/home/lfp/.gnupg-test/pubring.kbx
---------------------------------
sec   rsa4096/0x2954C91DEAE3C032 2020-05-03 [C] [有效至：2020-05-04]
      密钥指纹 = 76D1 39A8 F917 2972 195C  F33C 2954 C91D EAE3 C032
uid                   [ 绝对 ] usmile <usmile@wx.com>
uid                   [ 绝对 ] usmile <usmile@qq.com>
ssb   rsa3072/0x6E3823DE57F89766 2020-05-03 [S] [有效至：2020-05-04]
ssb   rsa3072/0x2CA24BFA9B038328 2020-05-03 [A] [有效至：2020-05-04]
```

### 删除密钥

删除整个密钥，而非子密钥

```bash
lfp@legion:~$ gpg --homedir ~/.gnupg-test/ -K
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
/home/lfp/.gnupg-test/pubring.kbx
---------------------------------
sec   rsa4096/0x2954C91DEAE3C032 2020-05-03 [C] [已过期：2020-05-04]
      密钥指纹 = 76D1 39A8 F917 2972 195C  F33C 2954 C91D EAE3 C032
uid                   [已过期] usmile <usmile@wx.com>
uid                   [已过期] usmile <usmile@qq.com>

sec   rsa4096/0x70105702D00EB8F4 2020-05-04 [C] [已吊销：2020-05-04]
      密钥指纹 = 0B19 E909 EB7D 0060 6615  693F 7010 5702 D00E B8F4
uid                   [已吊销] usmile <usmile@qq.com>
# 删除密钥
lfp@legion:~$ gpg --homedir ~/.gnupg-test/ --delete-secret-and-public-key 0x70105702D00EB8F4
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg (GnuPG) 2.2.4; Copyright (C) 2017 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.


sec  rsa4096/0x70105702D00EB8F4 2020-05-04 usmile <usmile@qq.com>

要从钥匙环里删除这把密钥吗？(y/N) y
这是一把私钥！――真的要删除吗？(y/N) y
pub  rsa4096/0x70105702D00EB8F4 2020-05-04 usmile <usmile@qq.com>

要从钥匙环里删除这把密钥吗？(y/N) y
# 已经删除
lfp@legion:~$ gpg --homedir ~/.gnupg-test/ -K
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
/home/lfp/.gnupg-test/pubring.kbx
---------------------------------
sec   rsa4096/0x2954C91DEAE3C032 2020-05-03 [C] [已过期：2020-05-04]
      密钥指纹 = 76D1 39A8 F917 2972 195C  F33C 2954 C91D EAE3 C032
uid                   [已过期] usmile <usmile@wx.com>
uid                   [已过期] usmile <usmile@qq.com>
```

#### 公私钥单独删除

```bash
# 无法先删除公钥
lfp@legion:~$ gpg --homedir ~/.gnupg-test/ --delete-keys 0x2954C91DEAE3C032
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg (GnuPG) 2.2.4; Copyright (C) 2017 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

gpg: 公钥“0x2954C91DEAE3C032”有对应的私钥！
gpg: 请先使用“--delete-secret-keys”选项来删除它。

# 先删除私钥部分才能删除公钥部分
lfp@legion:~$ gpg --homedir ~/.gnupg-test/ --delete-secret-keys 0x2954C91DEAE3C032
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg (GnuPG) 2.2.4; Copyright (C) 2017 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.


sec  rsa4096/0x2954C91DEAE3C032 2020-05-03 usmile <usmile@wx.com>

要从钥匙环里删除这把密钥吗？(y/N) y
这是一把私钥！――真的要删除吗？(y/N) y
```

### 加密、解密

```bash
lfp@legion:~/bin$ gpg -k
/home/lfp/.gnupg/pubring.kbx
----------------------------
pub   rsa3072/0x16A3B99FBAC538AE 2020-05-01 [SC] [已吊销：2020-05-04]
      密钥指纹 = 1256 45C3 0AAB 72BB D113  4A5F 16A3 B99F BAC5 38AE
uid                   [已吊销] lfp1024 <lfp1024@126.com>

pub   rsa4096/0x92C4D0C89EAA7333 2020-05-04 [SC] [有效至：2020-05-05]
      密钥指纹 = C95B 86BC 7919 F706 8CD6  2B08 92C4 D0C8 9EAA 7333
uid                   [ 绝对 ] usmile <usmile@qq.com>
sub   rsa4096/0x918087E1E6101F9D 2020-05-04 [E] [有效至：2020-05-05]

lfp@legion:~/bin$ gpg --recipient 0x918087E1E6101F9D --output ~/bin/test.en.txt --encrypt test.txt
lfp@legion:~/bin$ gpg --decrypt test.en.txt --output test.de.txt
gpg: Note: '--output' is not considered an option
usage: gpg [options] --decrypt [filename]
lfp@legion:~/bin$ gpg --output test.de.txt --decrypt test.en.txt
gpg: 由 4096 位的 RSA 密钥加密，钥匙号为 0x918087E1E6101F9D、生成于 2020-05-04
      “usmile <usmile@qq.com>”
```

### 签名

通过默认配置生成的主密钥具有[SC]两种功能，如果同时存在一个具有[S]功能的子密钥，则会通过子密钥去签名

实际是通过密钥的私钥部分去签名，然后通过该密钥的公钥部分去验证签名

```bash
# 在当前目录生成一份包含文件内容的签名文件 sign-test.Release.gpg 
lfp@legion:~$ gpg --homedir ~/.gnupg-test --sign sign-test.Release 
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'

# 在当前目录生成一份包含文件内容的签名文件 sign-test.Release.asc
lfp@legion:~$ gpg --homedir ~/.gnupg-test --clearsign sign-test.Release
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'

# 在当前目录生成一份【不包含】文件内容的签名文件 sign-test.Release.sig
lfp@legion:~$ gpg --homedir ~/.gnupg-test --detach-sign sign-test.Release
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'

# 在当前目录生成一份【不包含】文件内容的签名文件 sign-test.Release.asc
lfp@legion:~$ gpg --homedir ~/.gnupg-test --detach-sign --armor sign-test.Release
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'


# 可以通过 --output 指定路径及文件名，但是需要注意参数的位置，要放在 --sign 之前
lfp@legion:~$ gpg --homedir ~/.gnupg-test --output ~/bin/sign-test.Release.gpg --sign sign-test.Release
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
lfp@legion:~$ gpg --homedir ~/.gnupg-test --output ~/bin/sign-test.Release.asc --clearsign sign-test.Release
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
```

### 验证

1. 通过 gpgv 工具
2. 通过 gpg

```bash
lfp@legion:~$ gpg --homedir ~/.gnupg-test -k
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
/home/lfp/.gnupg-test/pubring.kbx
---------------------------------
pub   rsa4096/0x2954C91DEAE3C032 2020-05-03 [C] [有效至：2020-05-04]
      密钥指纹 = 76D1 39A8 F917 2972 195C  F33C 2954 C91D EAE3 C032
uid                   [ 绝对 ] usmile <usmile@wx.com>
uid                   [ 绝对 ] usmile <usmile@qq.com>
sub   rsa3072/0x6E3823DE57F89766 2020-05-03 [S] [有效至：2020-05-04]
sub   rsa3072/0x2CA24BFA9B038328 2020-05-03 [A] [有效至：2020-05-04]

# gpgv 需要通过 --keyring 指定密钥环
lfp@legion:~$ gpgv --keyring ~/.gnupg-test/pubring.kbx sign-test.Release.gpg 
gpgv: 签名建立于 2020年05月04日 星期一 19时01分58秒 CST
#  后16位即使用的密钥ID 0x6E3823DE57F89766
gpgv:                使用 RSA 密钥 92E43802C92A69A4F7B069E86E3823DE57F89766
gpgv: 完好的签名，来自于“usmile <usmile@wx.com>”
# 因为有两个uid
gpgv:                 亦即“usmile <usmile@qq.com>”

# gpg 只需要指出homedir 即可
# 通过gpg验证可以检查出密钥是否过期
lfp@legion:~$ gpg --homedir ~/.gnupg-test/ --verify sign-test.Release.gpg 
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg: 签名建立于 2020年05月04日 星期一 19时01分58秒 CST
gpg:                使用 RSA 密钥 92E43802C92A69A4F7B069E86E3823DE57F89766
gpg: 完好的签名，来自于“usmile <usmile@wx.com>” [已过期]
gpg:                 亦即“usmile <usmile@qq.com>” [已过期]
gpg: 注意：这把密钥已经过期了！
主钥指纹： 76D1 39A8 F917 2972 195C  F33C 2954 C91D EAE3 C032
     子钥指纹： 92E4 3802 C92A 69A4 F7B0  69E8 6E38 23DE 57F8 9766
########################################################################

lfp@legion:~/.gnupg$ gpg -k
/home/lfp/.gnupg/pubring.kbx
----------------------------
pub   rsa3072/0x16A3B99FBAC538AE 2020-05-01 [SC] [已吊销：2020-05-04]
      密钥指纹 = 1256 45C3 0AAB 72BB D113  4A5F 16A3 B99F BAC5 38AE
uid                   [已吊销] lfp1024 <lfp1024@126.com>
# 无法判定密钥的性质
lfp@legion:~/.gnupg$ gpgv --keyring ~/.gnupg/pubring.kbx ~/bin/test.txt.gpg 
gpgv: 签名建立于 2020年05月04日 星期一 19时17分25秒 CST
gpgv:                使用 RSA 密钥 3142022C68CD0B034DBAA5A0CF8C3097514C5331
gpgv: 完好的签名，来自于“lfp1024 <lfp1024@126.com>”

# 通过gpg验证可以检查出密钥是否被吊销
lfp@legion:~/.gnupg$ gpg --verify ~/bin/test.txt.gpg 
gpg: 签名建立于 2020年05月04日 星期一 19时17分25秒 CST
gpg:                使用 RSA 密钥 3142022C68CD0B034DBAA5A0CF8C3097514C5331
gpg: 完好的签名，来自于“lfp1024 <lfp1024@126.com>” [绝对]
gpg: 警告：这把密钥已经被它的持有者吊销了！
gpg:          这表明这个签名有可能是伪造的。
gpg: 吊销原因： 密钥不再使用
主钥指纹： 1256 45C3 0AAB 72BB D113  4A5F 16A3 B99F BAC5 38AE
     子钥指纹： 3142 022C 68CD 0B03 4DBA  A5A0 CF8C 3097 514C 5331
```

通过`gpg --keyring`验证，测试不行

```bash
# 测试发现，不能正确验证，无法识别密钥
lfp@legion:~/.gnupg-test$ gpg --keyring ~/.gnupg-test/pubring.kbx --no-default-keyring --verify ~/sign-test.Release.asc
gpg: assuming signed data in '/home/lfp/sign-test.Release'
gpg: 签名建立于 2020年05月04日 星期一 21时45分27秒 CST
gpg:                使用 RSA 密钥 5C886AA3B13C8CD76A453E6517B875E5EB1FA9D1
gpg: 完好的签名，来自于“usmile <usmile@qq.com>” [未知]
gpg: 警告：这把密钥未经受信任的签名认证！
gpg:          没有证据表明这个签名属于它所声称的持有者。
主钥指纹： 0B19 E909 EB7D 0060 6615  693F 7010 5702 D00E B8F4
     子钥指纹： 5C88 6AA3 B13C 8CD7 6A45  3E65 17B8 75E5 EB1F A9D1
     
lfp@legion:~/.gnupg-test$ gpg --homedir ~/.gnupg-test/ --verify ~/sign-test.Release.asc
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg: assuming signed data in '/home/lfp/sign-test.Release'
gpg: 签名建立于 2020年05月04日 星期一 21时45分27秒 CST
gpg:                使用 RSA 密钥 5C886AA3B13C8CD76A453E6517B875E5EB1FA9D1
gpg: 完好的签名，来自于“usmile <usmile@qq.com>” [绝对]
主钥指纹： 0B19 E909 EB7D 0060 6615  693F 7010 5702 D00E B8F4
     子钥指纹： 5C88 6AA3 B13C 8CD7 6A45  3E65 17B8 75E5 EB1F A9D1
```

### 吊销整个密钥

#### 生成吊销证书

```bash
# 生成吊销证书
# 通过 --output 配置项指定生成文件目录及名称，注意顺序在前
lfp@legion:~$ gpg --homedir ~/.gnupg-test/ --output ~/.gnupg-test/rev.asc --gen-revoke 0x70105702D00EB8F4
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'

sec  rsa4096/0x70105702D00EB8F4 2020-05-04 usmile <usmile@qq.com>

要为这把密钥建立一份吊销证书吗？(y/N) y
请选择吊销的原因：
  0 = 未指定原因
  1 = 密钥已泄漏
  2 = 密钥被替换
  3 = 密钥不再使用
  Q = 取消
(也许您会想要在这里选择 1)
您的决定是什么？ 3
请输入描述(可选)；以空白行结束：
> 
吊销原因：密钥不再使用
(不给定描述)
这样可以吗？ (y/N) y
已强行使用 ASCII 封装过的输出。
已建立吊销证书。

请把这个文件转移到一个可隐藏起来的介质(如软盘)上；如果坏人能够取得这
份证书的话，那么他就能让您的密钥无法继续使用。把这份凭证打印出来再藏
到安全的地方也是很好的方法，以免您的保存媒体损毁而无法读取。但是千万
小心：您的机器上的打印系统可能会在打印过程中把这些数据临时在某个其他
人也能够看得到的地方！
```

#### 导入吊销证书

```bash
# 导入并吊销整个密钥
lfp@legion:~$ gpg --homedir ~/.gnupg-test/ --import ~/.gnupg-test/rev.asc 
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg: 密钥 0x70105702D00EB8F4：“usmile <usmile@qq.com>”吊销证书已被导入
gpg: 合计被处理的数量：1
gpg:    新的密钥吊销：1
gpg: marginals needed: 3  completes needed: 1  trust model: pgp
gpg: 深度：0 有效性：  2 已签名：  0 信任度：0-，0q，0n，0m，0f，2u
gpg: 下次信任度数据库检查将于 2020-05-06 进行
```

###### 使用预先生成的吊销证书

```bash
lfp@legion:~$ gpg -k
/home/lfp/.gnupg/pubring.kbx
----------------------------
pub   rsa3072/0x16A3B99FBAC538AE 2020-05-01 [SC] [已吊销：2020-05-04]
      密钥指纹 = 1256 45C3 0AAB 72BB D113  4A5F 16A3 B99F BAC5 38AE
uid                   [已吊销] lfp1024 <lfp1024@126.com>

pub   rsa4096/0x92C4D0C89EAA7333 2020-05-04 [SC] [有效至：2020-05-05]
      密钥指纹 = C95B 86BC 7919 F706 8CD6  2B08 92C4 D0C8 9EAA 7333
uid                   [ 绝对 ] usmile <usmile@qq.com>
sub   rsa4096/0x918087E1E6101F9D 2020-05-04 [E] [有效至：2020-05-05]
# 找不到。。。
lfp@legion:~$ gpg --import ~/.gnupg/openpgp-revocs.d/C95B86BC7919F7068CD62B0892C4D0C89EAA7333.rev 
gpg: 找不到有效的 OpenPGP 数据。
gpg: 合计被处理的数量：0
```

#### 上传至公钥服务器

```bash
# Search your key on the key-server
gpg --keyserver <服务器地址> --search-keys <key-ID>
# Send the revoked key to the key-server
gpg --keyserver <服务器地址> --send-keys <key-ID>
```

> https://superuser.com/questions/1526283/how-to-revoke-a-gpg-key-and-upload-in-gpg-server

### 吊销子密钥

吊销某个uid同理，用`uid 1`选定某个uid，通过`revuid`吊销

`-delkey` 只删除密钥的公共部分，如果已经上传到公钥服务器，则需要使用`-revkey`吊销并上传公钥服务器

> https://www.gnupg.org/gph/en/manual/c235.html

```bash
lfp@legion:~/.gnupg-test$ gpg --homedir ~/.gnupg-test/ --expert --edit-key 0x70105702D00EB8F4
gpg: WARNING: unsafe permissions on homedir '/home/lfp/.gnupg-test'
gpg (GnuPG) 2.2.4; Copyright (C) 2017 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

私钥可用。

sec  rsa4096/0x70105702D00EB8F4
     创建于：2020-05-04  有效至：2020-05-06  可用于：C   
     信任度：绝对        有效性：绝对
ssb  rsa3072/0x17B875E5EB1FA9D1
     创建于：2020-05-04  有效至：2020-05-06  可用于：S   
[ 绝对 ] (1). usmile <usmile@qq.com>

# 选定某个子密钥，下标从1开始
gpg> key 1

sec  rsa4096/0x70105702D00EB8F4
     创建于：2020-05-04  有效至：2020-05-06  可用于：C   
     信任度：绝对        有效性：绝对
# 选定该子密钥
ssb* rsa3072/0x17B875E5EB1FA9D1
     创建于：2020-05-04  有效至：2020-05-06  可用于：S   
[ 绝对 ] (1). usmile <usmile@qq.com>

gpg> revkey
您真的要吊销这把子钥吗？(y/N) y
请选择吊销的原因：
  0 = 未指定原因
  1 = 密钥已泄漏
  2 = 密钥被替换
  3 = 密钥不再使用
  Q = 取消
您的决定是什么？ 3
请输入描述(可选)；以空白行结束：
> 
吊销原因：密钥不再使用
(不给定描述)
这样可以吗？ (y/N) y

sec  rsa4096/0x70105702D00EB8F4
     创建于：2020-05-04  有效至：2020-05-06  可用于：C   
     信任度：绝对        有效性：绝对
The following key was revoked on 2020-05-04 by RSA key 0x70105702D00EB8F4 usmile <usmile@qq.com>
ssb  rsa3072/0x17B875E5EB1FA9D1
     创建于：2020-05-04  已吊销：2020-05-04  可用于：S   
[ 绝对 ] (1). usmile <usmile@qq.com>

gpg>  save
```