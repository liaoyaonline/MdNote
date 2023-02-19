# 将日志转发默认设置为禁用(暂时不要MR，改好后和玉喜哥说一下)

## 问题详情

http://10.0.1.8/mantis/view.php?id=63059

![image-20211129102823995](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211129102823995.png)

## 问题复现

省略

## 问题追踪

```
kssl/GAD/php/web/module/sys/logger/model/SyslogModel.php//追查到这里的$this->setting = $this->config->getForwardSetting();已经设置好默认值了
```

![image-20211129144218693](将日志转发默认设置为禁用(暂时不要MR，改好后和玉喜哥说一下).assets/image-20211129144218693.png)

追查`getForwardSetting()`函数，这个函数使用`public function XMLConfig::getProperties($name, $position = 0, $comparator = null) array|null`来获取上述数据

![image-20211129154227250](将日志转发默认设置为禁用(暂时不要MR，改好后和玉喜哥说一下).assets/image-20211129154227250.png)

查看`XMLConfig`中对应的函数，发现其中有函数为`getXmlPath`获取其解析的`xml`文件路径，调用该函数

打印路径

![image-20211129154807460](将日志转发默认设置为禁用(暂时不要MR，改好后和玉喜哥说一下).assets/image-20211129154807460.png)

打开路径文件，看到默认设置的配置

![image-20211129155009016](将日志转发默认设置为禁用(暂时不要MR，改好后和玉喜哥说一下).assets/image-20211129155009016.png)

## 问题解决

将`On`修改为`Off`

![image-20211129155109151](将日志转发默认设置为禁用(暂时不要MR，改好后和玉喜哥说一下).assets/image-20211129155109151.png)

## 测试验证

![image-20211129155134436](将日志转发默认设置为禁用(暂时不要MR，改好后和玉喜哥说一下).assets/image-20211129155134436.png)

