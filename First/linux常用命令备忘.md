# linux常用命令备忘
- pkill -f trojan
- du -sh * //查看文件中大小
- fdisk -l //查看硬盘各个表的情况
- locate kcg//寻找名字包含kcg的文件
- df -TH //查看已经挂载的表的情况
- mkfs -t ext4 -c /dev/sdb1//将指定硬盘格式化，必须先卸载
- blkid //查看硬盘的UUID
- mount /dev/sdb1 /mnt //将/dev/sdb1分区挂载到/mnt文件夹下面	
- fdisk /dev/sdb //创建sdb里面的新分区
- /etc/fstab //自动挂载文件
- find / -name "a.c"//寻找根目录中名为a.c的文件
find /home/liaoya/github/novel-plus/ -name '*' | xargs grep '排行' > log //查找该目录下所有包含排行字符的文件
- sudo dpkg -i XXXX.deb//安装deb文件命令
- unrar x etc.rar //将etc.rar按照路径解压
- unrar e -p etc.rar //解压带密码的rar
- 7za x 111.7z -p123456//解压带密码的7z
- unzip -O gbk 2541.txt.zip//解压带有GBK模式的zip，可以省略解压正常模式zip
- gedit --encoding GBK 马恩的日常.txt//选择使用GBK解码模式打开马恩的日常.txt
- sudo tar -jxvf filename.tar.bz2 -C FilePath//将文件filename解压到FilePath里面
- tar -jxvf  filename.tar.bz2//解压tar.bz2文件
- tar -zxvf studio.tar.gz//解压studio.tar.gz
- sudo aria2c --conf-path=/etc/aria2/aria2.conf //启动aria2
- sudo apt install -f//安装时缺少依赖，此命令会自动安装缺少的依赖
- chmod 777 filename.js //设置文件权限为所有用户读写操作
- sudo chmod 777 * -R eladmin//设置文件夹eladmin内的所有文件读写操作
- kill -9 PID//杀死PID为PID的进程
- awk '{$1="";print $0}' Treedata.txt//删除Treedata.txt的第一列
- %s/\r//g //替换所有\r符号为无
- :%s/ //g//去除空格；
- :%s/\n//g//去除换行
- %s/s/b/gc//将所有s替换成b，且手确认
- awk '{for(i=2;i<=NF;++i) printf $i "\t";printf "\n"}' test.txt//打印除了第一列的其他列
- dot -Tpdf treeone.dot -o treeone.pdf//将treeone.dot转化为图像
- lm //在引用math.h库时，需要在编译命令后面加-lm
- tree -L 1//将当前目录的文件用树状图表示出来
- trans -shell -brief//使用trans进行翻译
- sudo mount -t cifs -o username=liaoya,password=123 //192.168.122.67/Downloadsall /data/tmp//将某ip地址的共享文件夹(windows10)挂载到某个文件夹(linux)下面，目的是方便传递文件
- sudo mount -a//让挂载文件/etc/fstab文件生效
-  sudo apt-get --purge remove electron-ssr//卸载名为electron-ssr的程序

## 其他

使用`traceroute`跟踪路由跳转，其命令如下

```
traceroute 10.0.40.73
```



```
address//ip地址
network//网段地址
netmask//子网掩码
broadcast//广播地址
```



扫描指定端口

```
sudo nmap -sS -p 22 10.0.40.72
```





查询文件内包含`hmac`字符的句子，并输出序号

```
egrep "hmac" -nr
```



使用`file`命令可以查看文件的编码方式

使用`hexdump`来查看转换编码后hex有没有发生变化

转换编码格式

```
iconv -f gbk -t utf-8 testcert_gbk.cnf > testcert_utf8.cnf
```

在xftp连接到linux上时，将属性里面的编码设置为utf-8，不然中文文件从windows上传过去会变成乱码







```
-Wall 编译时会显示所有警告，建议带上
```



```
b fko_encrypt_spa_data //直接设置断点为函数
step //跳转到函数内
info addrfko_encrypt_spa_data //查看函数的地址空间
```





```
ldd a.out
```

查看应用文件链接的库

- Set -e 

  你写的每个脚本都应该在文件开头加上set -e,这句语句告诉bash如果任何语句的执行结果不是true则应该退出。
  这样的好处是防止错误像滚雪球般变大导致一个致命的错误，而这些错误本应该在之前就被处理掉。如果要增加可读性，可以使用set -o errexit，它的作用与set -e相同。

- 2>&1

  ```text
  ./test.sh  > log.txt 2>&1
  ```

  这里的2>&1是什么意思？该如何理解？
  先说结论：上面的调用表明将./test.sh的输出重定向到log.txt文件中，同时将标准错误也重定向到log.txt文件中。

  

  

  每个程序在运行后，都会至少打开三个文件描述符，分别是0：标准输入；1：标准输出；2：标准错误。
  例如，对于前面的test.sh脚本，我们通过下面的步骤看到它至少打开了三个文件描述符：

  ```text
  ./test.sh    #运行脚本
  ps -ef|grep test.sh  #重新打开命令串口，使用ps命令找到test.sh的pid
  hyb       5270  4514  0 19:20 pts/7    00:00:00 /bin/bash ./test.sh
  hyb       5315  5282  0 19:20 pts/11   00:00:00 grep --color=auto test.sh
  ```

  可以看到test.sh的pid为5270，进入到相关fd目录：

  ```text
  cd /proc/5270/fd   #进程5270所有打开的文件描述符信息都在此
  ls -l              #列出目录下的内容
   0 -> /dev/pts/7
   1 -> /dev/pts/7
   2 -> /dev/pts/7
   255 -> /home/hyb/workspaces/shell/test.sh
  ```

  可以看到，test.sh打开了0，1，2三个文件描述符。同样的，如果有兴趣，也可以查看其他运行进程的文件描述符打开情况，除非关闭了否则都会有这三个文件描述符。

  那么现在就容易理解前面的疑问了，2>&1表明将文件描述2（标准错误输出）的内容重定向到文件描述符1（标准输出），为什么1前面需要&？当没有&时，1会被认为是一个普通的文件，有&表示重定向的目标不是一个文件，而是一个文件描述符。在前面我们知道，test.sh >log.txt又将文件描述符1的内容重定向到了文件log.txt，那么最终标准错误也会重定向到log.txt。我们同样通过前面的方法，可以看到test.sh进程的文件描述符情况如下：

  ```text
   0 -> /dev/pts/7
   1 -> /home/hyb/workspaces/shell/log.txt
   2 -> /home/hyb/workspaces/shell/log.txt
   255 -> /home/hyb/workspaces/shell/test.sh
  ```

  我们可以很明显地看到，**文件描述符1和2都指向了log.txt文件，**也就得到了我们最终想要的效果：**将标准错误输出重定向到文件中**。
  它们还有两种等价写法：

  ```text
  ./test.sh  >& log.txt
  ./test.sh  &> log.txt
  ```

