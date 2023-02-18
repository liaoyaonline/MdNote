# Docker学习

## 安装Docker

安装完docker后，执行docker相关命令，报错

```
”Got permission denied while trying to connect to the Docker daemon socket at unix:///var/run/docker.sock: Get http://%2Fvar%2Frun%2Fdocker.sock/v1.26/images/json: dial unix /var/run/docker.sock: connect: permission denied“
```

大概的意思就是：docker进程使用Unix Socket而不是TCP端口。而默认情况下，Unix socket属于root用户，需要root权限才能访问。

**解决方法1**

使用sudo获取管理员权限，运行docker命令

**解决方法2**

docker守护进程启动的时候，会默认赋予名字为docker的用户组读写Unix socket的权限，因此只要创建docker用户组，并将当前用户加入到docker用户组中，那么当前用户就有权限访问Unix socket了，进而也就可以执行docker相关命令

```
sudo groupadd docker     #添加docker用户组
sudo gpasswd -a $USER docker     #将登陆用户加入到docker用户组中
newgrp docker     #更新用户组
docker ps    #测试docker命令是否可以使用sudo正常使用
```

![image-20211116172406755](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116172406755.png)

## Docker命令测试

### Docker Hello World

```shell
docker run ubuntu:15.10 /bin/echo "Hello world"
```

各个参数解析：

- **docker:** Docker 的二进制执行文件。
- **run:** 与前面的 docker 组合来运行一个容器。
- **ubuntu:15.10** 指定要运行的镜像，Docker 首先从本地主机上查找镜像是否存在，如果不存在，Docker 就会从镜像仓库 Docker Hub 下载公共镜像。
- **/bin/echo "Hello world":** 在启动的容器里执行的命令

以上命令完整的意思可以解释为：Docker 以 ubuntu15.10 镜像创建一个新容器，然后在容器里执行 bin/echo "Hello world"，然后输出结果。

![image-20211116173704803](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116173704803.png)

### 运行交互式的容器

我们通过 docker 的两个参数 -i -t，让 docker 运行的容器实现**"对话"**的能力：

```
runoob@runoob:~$ docker run -i -t ubuntu:15.10 /bin/bash
root@0123ce188bd8:/#
```

各个参数解析：

- **-t:** 在新容器内指定一个伪终端或终端。
- **-i:** 允许你对容器内的标准输入 (STDIN) 进行交互。

注意第二行 **root@9099ea7e7209:/#**，此时我们已进入一个 ubuntu15.10 系统的容器

我们尝试在容器中运行命令 **cat /proc/version**和**ls**分别查看当前系统的版本信息和当前目录下的文件列表

![image-20211116174045060](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116174045060.png)

## 启动容器（后台模式）

使用以下命令创建一个以进程方式运行的容器

```
liaoya@ubuntu:~$ docker run -d ubuntu:15.10 /bin/sh -c "while true; do echo hello world; sleep 1; done"
a57d7d593994734e3bc217cbdb420df3d050dfefbccb82c267db6dbfcf583b23
```

在输出中，我们没有看到期望的 "hello world"，而是一串长字符

**a57d7d593994734e3bc217cbdb420df3d050dfefbccb82c267db6dbfcf583b23**

这个长字符串叫做容器 ID，对每个容器来说都是唯一的，我们可以通过容器 ID 来查看对应的容器发生了什么。

首先，我们需要确认容器有在运行，可以通过 **docker ps** 来查看：

```
liaoya@ubuntu:~$ docker ps
CONTAINER ID   IMAGE          COMMAND                  CREATED          STATUS          PORTS     NAMES
a57d7d593994   ubuntu:15.10   "/bin/sh -c 'while t…"   49 seconds ago   Up 48 seconds             hardcore_pascal
```

输出详情介绍：

**CONTAINER ID:** 容器 ID。

**IMAGE:** 使用的镜像。

**COMMAND:** 启动容器时运行的命令。

**CREATED:** 容器的创建时间。

**STATUS:** 容器状态。

状态有7种：

- created（已创建）
- restarting（重启中）
- running 或 Up（运行中）
- removing（迁移中）
- paused（暂停）
- exited（停止）
- dead（死亡）

**PORTS:** 容器的端口信息和使用的连接类型（tcp\udp）。

**NAMES:** 自动分配的容器名称。

在宿主主机内使用 **docker logs** 命令，查看容器内的标准输出：

```
liaoya@ubuntu:~$ docker logs a57d7d593994

```

![image-20211116174737403](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116174737403.png)

------

## 停止容器

我们使用 **docker stop** 命令来停止容器:

![img](https://www.runoob.com/wp-content/uploads/2016/05/docker25.png)

通过 **docker ps** 查看，容器已经停止工作:

```
runoob@runoob:~$ docker ps
```

可以看到容器已经不在了。

也可以用下面的命令来停止:

```
runoob@runoob:~$ docker stop amazing_cori
```

## 列出镜像

要想列出已经下载下来的镜像，可以使用 `docker image ls` 命令。

```
root@ubuntu:/home/liaoya/github/http-testpage# docker image ls
REPOSITORY    TAG          IMAGE ID       CREATED       SIZE
ubuntu        latest       ba6acccedd29   4 weeks ago   72.8MB
hello-world   latest       feb5d9fea6a5   7 weeks ago   13.3kB
php           5.6-apache   24c791995c1e   2 years ago   355MB
ubuntu        15.10        9b9cb95443b5   5 years ago   137MB
```

列表包含了 `仓库名`、`标签`、`镜像 ID`、`创建时间` 以及 `所占用的空间`。

其中仓库名、标签在之前的基础概念章节已经介绍过了。**镜像 ID** 则是镜像的唯一标识，一个镜像可以对应多个 **标签**。因此，在上面的例子中，我们可以看到 `ubuntu:18.04` 和 `ubuntu:latest` 拥有相同的 ID，因为它们对应的是同一个镜像。

## 删除本地镜像

如果要删除本地的镜像，可以使用 `docker image rm` 命令，其格式为：

```
$ docker image rm [选项] <镜像1> [<镜像2> ...]
```

运行实例

```
docker image rm 24c791995c1e
Untagged: php:5.6-apache
Untagged: php@sha256:0a40fd273961b99d8afe69a61a68c73c04bc0caa9de384d3b2dd9e7986eec86d
Deleted: sha256:24c791995c1e498255393db857040793a7a040fa0f8ddd4bbe8fb230648e37d7
Deleted: sha256:18bd948e6c9a13afe3933b1d059b651e627382d80f6ae72d3bb4da025b288b91
Deleted: sha256:03f3309f3e0ea9e681ee3613407a6dde05930091c6d13a7c6fbe142f6db90e0c
Deleted: sha256:74f1b90f173a93506c437858eebbf86e7a1d96b0c010707c03d954e26b44ee29
Deleted: sha256:90eafedb4f7d22732ac3ff8fb8a31ebbdbce14c5e5371dab26eecd35f4e7a44c
Deleted: sha256:9b48bfe7263ff0f50186d8971c32e67f496109eaaa533c5a7f48b1f748994e00
```

列出所有在运行的容器信息。

```
root@ubuntu:/home/liaoya/github/http-testpage# docker ps
CONTAINER ID   IMAGE          COMMAND                  CREATED          STATUS          PORTS                               NAMES
f560d6e6b2a7   nginx          "/docker-entrypoint.…"   12 seconds ago   Up 10 seconds   0.0.0.0:80->80/tcp, :::80->80/tcp   rmimage
a57d7d593994   ubuntu:15.10   "/bin/sh -c 'while t…"   41 hours ago     Up 41 hours                                         hardcore_pascal
```

输出详情介绍：

**CONTAINER ID:** 容器 ID。

**IMAGE:** 使用的镜像。

**COMMAND:** 启动容器时运行的命令。

**CREATED:** 容器的创建时间。

**STATUS:** 容器状态。

**PORTS:** 容器的端口信息和使用的连接类型（tcp\udp）。

**NAMES:** 自动分配的容器名称。

## 运行指令详解

```shell
 docker run --name dockertest1 -p 8080:80 -d nginx
```

- --name dockertest1为容器指定一个名字为dockertest1
- -p 8080：80指定端口映射，格式为:主机(宿主)端口:容器端口，这里就是指定将主机的8080端口映射到容器的80端口
- -d 后台运行
- nginx 使用名为nginx的镜像创建容器
```shell
docker run --rm -p 8080:80 git.koal.com:4567/gw-server/docker/http-testpage
```

- --rm 结束运行时清空文件系统

- -p 8080:80 指定端口映射，将8080端口映射到容器的80端口
```
$ docker run --name webserver -d -p 80:80 nginx
```

- 指定运行一个名字为webserver的容器，后台运行(-d)，映射端口为80:80，在nginx镜像的基础上创建容器

