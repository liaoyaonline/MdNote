# 远程维护CA证书链勾选时的问题

## 问题详情

[0062015: 见附件 两套二级证书链 （其中有一个根跟另一个的二级重名） 导入后显示不对 - MantisBT](http://10.0.1.8/mantis/view.php?id=62015)

## 问题复现

进入远程维护界面，勾选bendism2时，会同时选中sm2root和sm2bendi

![image-20211117115524223](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117115524223.png)

## 问题追踪

`kssl/GAD/php/web/module/sys/maintain/remote.php`没有对js文件中判断的`issuerKeyIdentify`和`keyIdentify`进行赋值

![image-20211117135632990](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117135632990.png)

## 问题解决

将对应的参数进行定义赋值

![远程访问证书链出错代码](C:\Users\admin\Pictures\远程访问证书链问题\远程访问证书链出错代码.PNG)

## 测试验证



进入远程维护界面，勾选bendism2时，只会选中sm2bendi

![image-20211117134513374](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117134513374.png)

取消证书链根目录时，会同步取消其下级CA证书

![image-20211117134625901](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117134625901.png)