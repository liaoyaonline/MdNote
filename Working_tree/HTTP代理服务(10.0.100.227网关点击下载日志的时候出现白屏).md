# HTTP代理服务(10.0.100.227网关点击下载日志的时候出现白屏)

## 问题详情

![image-20211117113202488](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117113202488.png)

![image-20211117113217420](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117113217420.png)

## 问题复现

在`http`代理服务界面设置一个简单的反向代理到百度，此时后台节点显示`180.101.49.11:80`没通

![image-20211117113452726](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117113452726.png)

进入状态监控界面，点击打包下载，界面出现白屏

![image-20211117113828414](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117113828414.png)

## 问题追踪

找到代码`kssl/GAD/php/web/module/srv/nrp/down_log.c.php`，进行测试

不方便在100.227上改代码

配置导出，报错

![image-20211117165833512](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117165833512.png)



## 问题解决

## 测试验证

