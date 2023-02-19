# SP4证书链问题追踪

## 问题追踪

1，其他证书链显示的问题仍然没有解决，比如在创建服务的时候，选中二级证书bendism2的时候会同步选中sm2bendi（非root)，sm2root两个证书。

追查到`kssl/GAD/php/web/js/selectCert2.js`

发现其进行验证该证他的CA证书的代码

![image-20211110171417952](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211110171417952.png)

在进行断点测试时发现键值对`currentIssuerkey`是未定义的，这里应该保存着其签发证书的公钥hash值。

![image-20211110172355584](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211110172355584.png)

追查到`kssl/GAD/php/web/module/srv/nrp/cert_param.php`发现这里没有将`currentIssuerKey`取用的`tissuerkeyidentify`值进行定义

![image-20211110172744886](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211110172744886.png)

对`tissuerkeyidentify`值进行定义

![image-20211110173300305](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211110173300305.png)

能够看到已经在js中调用到`tissuerkeyidentify`值

![image-20211110173238175](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211110173238175.png)

结果发现只能单选，不能在选择二级证书的时候将其上级证书一并选择，在取消根证书的时候没有将其下级证书一并取消，继续追查源代码

![image-20211110171417952](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211110171417952.png)

发现`tkeyidentify`也是未定义的，将其一并在`kssl/GAD/php/web/module/srv/nrp/cert_param.php`中进行定义。定义后进行测试。

![image-20211111102927488](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211111102927488.png)

已经能够在选择二级证书的时候选择其对应的签发证书，取消根证书时取消其对应的二级证书。

