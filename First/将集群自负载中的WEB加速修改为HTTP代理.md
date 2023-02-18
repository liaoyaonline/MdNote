# 将集群自负载中的WEB加速修改为HTTP代理

## 问题详情

集群自负载的服务类型显示错误，WEB加速服务应该是HTTP代理服务

![image-20211117142357994](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117142357994.png)

## 问题追踪

`cfg/GAD/web/module/net/ha/resource.ini`中定义类型名为WEB加速服务

![image-20211117143148482](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117143148482.png)

## 问题解决

将WEB加速服务修改为HTTP代理服务

![image-20211117143554843](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117143554843.png)

## 测试验证

![image-20211117143538082](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211117143538082.png)