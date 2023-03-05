# Socket通信详解

[TOC]

发表地址:https://blog.csdn.net/liaoyaonline/article/details/129231091

## Socket流程介绍

socket通信类似于电话通信，其服务器基本流程就是

```flow
st=>start: 安装电话socket()
firstp=>operation: 分配电话号码bind()
secondp=>operation: 连接电话线listen()
e=>end: 拿起话筒accept()

st->firstp->secondp->e
```

## 函数介绍

- socket()

其中`socket`的函数原型如下所示，它的作用就是创建套接字，同时规定好该套接字的用途

![image-20230226113622696](Socket%E9%80%9A%E4%BF%A1%E8%AF%A6%E8%A7%A3.assets/image-20230226113622696.png)

其中的每个参数的作用如下所示：

```
domain:套接字使用的协议族信息(比如PF_INET就是使用IVP4互联网协议族)
type:套接字数据传输类型信息(比如SOCK_STREAM是指面向连接的套接字类型,TCP就是这个类型)
protocol:计算机通信中使用的协议信息(比如IPV4，面向连接类型，基本就只有TCP，这里就填写IPPROTO_TCP)
```

- bind()

其中`bind`的函数原型如下所示，它的作用就是把地址信息分配给套接字上，也就是前面`socket`生成的套接字上。其函数原型如下所示：

![image-20230226171720297](Socket%E9%80%9A%E4%BF%A1%E8%AF%A6%E8%A7%A3.assets/image-20230226171720297.png)

其中中间的`__CONST_SOCKADDR_ARG`是个宏，继续追踪可以知道是个存储地址信息的结构体，如下所示

![image-20230226171844729](Socket%E9%80%9A%E4%BF%A1%E8%AF%A6%E8%A7%A3.assets/image-20230226171844729.png)

其中每个参数的作用如下所示:

```
fd:套接字描述符，就是前文socket函数的返回值，结构体信息是绑定在这个套接字上的
addr:存储地址信息的指针，里面有IP和端口信息。
len:就是第二个参数addr的长度
```

那么`addr`里面的具体详情又是怎么样的呢，即`sockaddr`的结构体定义如下所示，是一个14个字节长度的字符串数组。

![image-20230226172932459](Socket%E9%80%9A%E4%BF%A1%E8%AF%A6%E8%A7%A3.assets/image-20230226172932459.png)

为了便于填写，我们一般使用`sockaddr_in`结构体，然后进行强制类型转换为`sockaddr`类型，

![image-20230226173232335](Socket%E9%80%9A%E4%BF%A1%E8%AF%A6%E8%A7%A3.assets/image-20230226173232335.png)

上图中的`name`就是`sockaddr_in`类型，`sockadr_in`结构体类型如下所示:

![image-20230226173507792](Socket%E9%80%9A%E4%BF%A1%E8%AF%A6%E8%A7%A3.assets/image-20230226173507792.png)

第一个红框中的变量为`sin_family`，其演变如下所示

![image-20230226174403952](Socket%E9%80%9A%E4%BF%A1%E8%AF%A6%E8%A7%A3.assets/image-20230226174403952.png)

对`socket_In`结构体的变量解析如下

```
sin_family:地址族（比如IPV4就填写AF_INET)
sin_port:填写16位网络端口，重点是它是以网络字节序保存，所以需要进行转换
sin_addr:填写32为ip地址，也以网络字节序保存。
sin_zero:无实际含义，为了保持与socket结构体长度一致，方便强制类型转换。
```

- listen()

其中`listen`的函数原型如下所示

![image-20230226175236357](Socket%E9%80%9A%E4%BF%A1%E8%AF%A6%E8%A7%A3.assets/image-20230226175236357.png)

其参数解析如下

```
fd:前面两个函数都用到过的套接字描述符
n:表示连接请求队列的长度，如果设置为5，则队列长度为5，表示最多使五个连接请求进入队列
```

## 编程实例

- 文件介绍

  `hello_server.c`就是主要的文件，也就是实例。`hello_client.c`是配合服务器测试的客户端程序。`Makefile`是用来编译两者的。

- 测试流程

  ```
  make
  ./hello_server 9190 //这个端口随便选择一个没有占用的端口即可
  ./hello_client 127.0.0.1 9190//这里的ip和端口号要与服务器的ip端口号保持一致
  ```

  此时客户端会收到一个消息，消息内容为`

- 测试截图

  服务端运行

  ![image-20230226202351901](Socket%E9%80%9A%E4%BF%A1%E8%AF%A6%E8%A7%A3.assets/image-20230226202351901.png)

  客户端运行以及结果

  ![image-20230226202424789](Socket%E9%80%9A%E4%BF%A1%E8%AF%A6%E8%A7%A3.assets/image-20230226202424789.png)

- hello_server.c

  ```c
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <sys/socket.h>
  #include <unistd.h>
  void error_handling(char* message);
  int main(int argc, char* argv[]) {
    int serv_sock;
    int clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;
    char message[] = "hello World!";
    if (argc != 2) {
      printf("Usage : %s <port>\n", argv[0]);
      exit(1);
    }
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);//创建套接字
    if (serv_sock == -1) {
      error_handling("socket() error");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
  
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)//将地址信息绑定套接字
      error_handling("bind() error");
    if (listen(serv_sock, 5) == -1) error_handling("listen() error");//监听套接字
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);//当收到客户端消息的时候创建一个套接字来进行通信
    if (clnt_sock == -1) {
      error_handling("accept() error");
    }
    write(clnt_sock, message, sizeof(message));
    close(clnt_sock);
    close(serv_sock);
    return 0;
  }
  void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
  }
  ```

  

- hello_client.c

  ```c
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <sys/socket.h>
  #include <unistd.h>
  void error_handling(char* message);
  int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char message[30];
    int str_len;
    if (argc != 3) {
      printf("Usage : %s <IP> <port>\n", argv[0]);
      exit(1);
    }
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
      error_handling("socket() error");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
  
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
      error_handling("connect() error");
    str_len = read(sock, message, sizeof(message) - 1);
    if (str_len == -1) error_handling("read() error");
    printf("Message from server: %s \n", message);
    close(sock);
    return 0;
  }
  void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
  }
  ```

  

- MakeFile

  ```makefile
  all: server client
  
  server: hello_server.c
  	clang-format -style=google -i hello_server.c
  	gcc hello_server.c -o hello_server
  client: hello_client.c
  	clang-format -style=google -i hello_client.c
  	gcc hello_client.c -o hello_client
  clean:
  	rm hello_client hello_server
  ```

  