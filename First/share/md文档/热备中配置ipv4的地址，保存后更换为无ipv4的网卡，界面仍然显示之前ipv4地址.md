# 热备中配置ipv4的地址，保存后更换为无ipv4的网卡，界面仍然显示之前ipv4地址

## 问题详情

在热备中先配置`ipv4`和`ipv6`地址都有的网卡，保存后修改为只有`ipv6`地址的网卡，在热备界面仍然显示出之前`ipv4`地址

## 问题复现

配置网络，将`eth2`配置为只有`ipv6`的网卡，将`eth3`配置为既有`ipv4`又有`ipv6`地址的网卡。

![image-20211125105123960](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211125105123960.png)

新建冗余热备服务，配置网卡和虚拟地址

![image-20211125105318904](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211125105318904.png)

保存后，返回界面，界面显示正常，点击修改，将配置修改为只有`ipv6`ip地址的网卡`eth2`，配置好虚拟地址。

![image-20211125105522712](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211125105522712.png)

返回热备管理界面，此时能看出不对来，明明没有配置`ipv4`地址，上面还是显示出了`ipv4`地址

![image-20211125105709240](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211125105709240.png)

问题复现！

## 问题追踪

在界面查看`js`传递的数据，发现没有将`ipv4`地址传递到对应的`xml`文件上。

![image-20211125110715312](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211125110715312.png)

打印`model`，其从配置文件中加载的数据能够看出配置文件中的`ipv6`地址仍然存在

![image-20211125112531296](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211125112531296.png)

## 问题解决

对`_REQUEST`增加判断，如果其传递到`xml`中的数据中有`ipv4`，那么不对加载后的`mode`进行处理，如果其传递到`xml`中的数据没有`ipv4`，那么对加载后的`mode`中的`ipv4`设置为空

![image-20211125112853895](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211125112853895.png)

## 测试验证

重新进行保存，能够看到`mode`里面`ip`的值是空的

![image-20211125113121815](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211125113121815.png)

同时界面上只显示刚才设置的`ipv6`地址

![image-20211125113212288](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211125113212288.png)