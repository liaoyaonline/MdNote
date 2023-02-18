# 修改SKS保存SM2缺少加密证书私钥导致出错

## 问题详情

[0062510: SKS服务证书链页面报错 日志和截图在附件 - MantisBT](http://10.0.1.8/mantis/view.php?id=62510)

![image-20211116150929954](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116150929954.png)

## 问题复现

证书管理界面导入一个双证书，在后台证书管理文件夹`cfg/GAD/cert/site`将其加密证书的key手动删除，得到一个和问题一样的缺少加密证书私钥的双证书

![image-20211115190334307](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211115190334307.png)

在sks界面将站点证书替换为该双证书，界面报错，该证书的私钥不存在

![image-20211115190509567](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211115190509567.png)

再次打开证书证书界面，界面上只有报错，不能进行操作，其报错如下，问题复现

![image-20211115190702960](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211115190702960.png)



## 问题追踪

根据报错信息追查到`kssl/GAD/php/web/module/cert/ca/model/ServiceCertModel2.php`

查到问题代码，代码并没有预先检测双证书的key是否存在，而是在后续进行检查，假如缺少签名证书的key，那么在运行时，检测到错误，链接失败。如果是缺少加密证书的key，那么签名证书是可以正常链接保存的。

![image-20211116143521755](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116143521755.png)

，然后到检测加密证书的时候会报错，导致保存的文件出现错误，刷新界面的时候会加载证书，然后只能加载错误的证书，然后报错。

![image-20211116144127930](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116144127930.png)

## 问题解决

在获取双证书加密证书和签名证书对应ID的时候，验证其是否存在对应的key文件，如果不存在报错，保存截止

![image-20211116142859434](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116142859434.png)

## 测试验证

当签名证书缺少私钥的时候，在SKS证书链环节选择SM2双证书，界面报错

![image-20211116143148100](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116143148100.png)

点击返回，回到证书链选择界面

![image-20211116141622314](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116141622314.png)

当双证书中的加密证书缺少私钥，在SKS证书选择环节，选择该双证书，点击保存，界面报错

![image-20211116143037118](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116143037118.png)

点击返回，能够返回证书链选择界面

![image-20211116143227735](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211116143227735.png)