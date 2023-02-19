# TCP新建服务没有在默认填写的端口号中填写之前服务的最大端口号加1

## 问题详情

http://10.0.1.8/mantis/view.php?id=62995

![image-20211126112634067](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211126112634067.png)

## 问题复现

乱序建立四个TCP服务，其端口乱序排列

![image-20211126112956957](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211126112956957.png)

新建服务时，其默认的端口号为`50447`也即是之前填写过的端口号

![image-20211126113128572](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211126113128572.png)

问题复现

## 问题追踪

`kssl/GAD/php/web/module/srv/shb2c/model/ServiceBasicParamModel.php`其逻辑为当`id`为空，并且`local_port`时会寻找路径`/cfg/SHB2C`上的默认`default`上的`basic.xml`文件中的端口，将这个端口加上当前的服务序数-1，如果是按照顺序排列，即default文件夹中的端口是344，那么默认第一个是端口345，第二个端口是346，默认第三个端口是347以此类推，但是当第二个端口填写60001时，默认第三个端口还是347。

![image-20211127100417361](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211127100417361.png)

## 问题解决

将逻辑进行修改，即找寻到`/cfg/SHB2C`文件夹上的所有服务，选择服务中的最大端口，返回最大端口号+1

![image-20211127103809441](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211127103809441.png)

## 测试验证

随机设置端口号，其随机设置如下所示

![image-20211127104251048](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211127104251048.png)

新建一个服务，会在默认填写的端口中填写前面服务端口号的最大值+1

![image-20211127104411348](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211127104411348.png)

