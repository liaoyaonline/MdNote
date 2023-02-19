# 修改SKS保存SM2缺少加密证书私钥导致出错

## 问题详情

[0062432: 虚拟地址配置 ipv6的地址去掉了 保存后启动界面显示还有 - MantisBT](http://10.0.1.8/mantis/view.php?id=62432)

![image-20211113161810566](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211113161810566.png)

## 问题复现

证书管理界面导入一个双证书，在后台证书管理文件夹`cfg/GAD/cert/site`将其加密证书的key手动删除，得到一个和问题一样的缺少加密证书私钥的双证书

![image-20211115190334307](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211115190334307.png)

在sks界面将站点证书替换为该双证书，界面报错，该证书的私钥不存在

![image-20211115190509567](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211115190509567.png)

再次打开证书证书界面，界面上只有报错，不能进行操作，其报错如下，问题复现

![image-20211115190702960](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211115190702960.png)



## 问题追踪

对SKS选中双证书后js传递的数据进行追踪，发现js只传递了双证书中签名证书的id，保存时也只验证了签名证书id有没有对应的私钥

![image-20211116104140933](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116104140933.png)

当签名证书不存在对应私钥时，界面报错，并且不能正常保存

![image-20211116104409833](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116104409833.png)











在代码中，函数通过判断ipv6中值是否为空来进行显示，在这里打印传过来的`data`数组。

![代码输出data数组](C:\Users\admin\Pictures\双机热备ipv6显示问题\代码输出data数组.PNG)

切换到界面，能够看到打印出来的`data`数组，能够看到`data`数组里面仍然保留着ipv6网卡的虚拟ipv6地址。

![界面显示data数据](C:\Users\admin\Pictures\双机热备ipv6显示问题\界面显示data数据.PNG)

打印从js界面传递到后台的数据，从数据上看，js界面没有将ipv6的值传递给后台，这个环节是正常的。

![image-20211114153201848](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211114153201848.png)

继续追踪保存这些数据的路径`/cfg/HA/cluster.conf.xml`文件，在这里发现里面保存的`ipv6`地址没有改变

![xml文件中的虚拟ipv6地址](C:\Users\admin\Pictures\双机热备ipv6显示问题\xml文件中的虚拟ipv6地址.PNG)

## 问题解决



在js数据传递到后台环节增加判断，如果传递的数据里面没有`ipv6`这个键，就将model里面的`ipv6`的值设置为空。

![image-20211114163511705](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211114163511705.png)

## 测试验证

在网络配置->高可用性->冗余热备界面添加一个无ipv6的网卡，设置好虚拟ipv4地址，界面显示正常

![image-20211113155429406](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211113155429406.png)

修改网卡为有ipv6网卡，界面显示正常

![image-20211113155543953](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211113155543953.png)

修改网卡为无ipv6网卡，界面显示正常

![image-20211113155638415](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211113155638415.png)

打印`data`数组的值，可以看到`ipv6`的取值为空



![修改后界面上没有将错误数据传递到前端显示](C:\Users\admin\Pictures\双机热备ipv6显示问题\修改后界面上没有将错误数据传递到前端显示.PNG)