# 对于两个相同的double类型的数相减结果不等于0解决方案
## 问题详情
在进行回归树生成的时候，发现叶节点上面出现诸如2.31112e-33的数字。经过不断的测试，发现错误代码是
```
for(int i = 0; i < Value_Class.size(); i++)
    {
        double x_tmp = (Value_Class[i] - Value_Average);
        double sum_tmp = x_tmp * x_tmp;
        Value_Variance += sum_tmp;
    }
```
其目的是计算Value_Class数组里面的每个值和它们的平均值之差的平方和。经过多次反复测试，确定Value_Class[i]里面的数值是0.2，Value_Average也是0.2，但是两者相减就是1.85145e-317。。。各种测试，测试得心态爆炸。。。下去玩了一小时炉石，心情平复。想起以前看过的一个知识点，计算机存储小数的机制，以及在计算机里面0.2可能不是0.2而是0.200000000198。仔细考量这个思路，觉得大概率就是这个问题了。在百度搜了下，发现是这个原因，解决方案就是人为设定精度，某个数的绝对值在多少范围内，就认为它是0。
## 解决方案
解决方案就是人为设定精度，(Value_Class[i] - Value_Average)的差在范围（-0.00001,0.00001）内，就认为它是0。以下是修改后的代码:
```
for(int i = 0; i < Value_Class.size(); i++)
    {
        double x_tmp = 0;
        if((Value_Class[i] - Value_Average) < 0.00001 && (Value_Class[i] - Value_Average) > -0.00001)
            x_tmp = 0;
        else
            x_tmp = (Value_Class[i] - Value_Average);
        double sum_tmp = x_tmp * x_tmp;
        Value_Variance += sum_tmp;
    }
```