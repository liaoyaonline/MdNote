# C++备忘录
## 一些理解
cout不需要指定格式，它会根据输出的格式自动输出。
## 常用函数
清空换行符
```
cin.ignore();//清空cin缓冲区的换行符
```
划分数据流
```
getline(cin, input_Line);
istringstream input_AttValues(input_Line);
for(int i = 0;i < data_Attributes.size();i++)
input_AttValues >> temp_AttValues;
```

返回大于等于某值的第一个元素
```
*s.lower_bound(4)
*s.upper_bound(3)//比３大的第一个元素
*s.find(3)//查找元素的迭代器
```

auto可以在声明变量的时候根据变量初始值的类型自动为此变量选择匹配的类型，类似的关键字还有decltype。举个例子：


```
int a =10;
auto i = a;//auto会自动匹配相应类型
```

```
a.clear()//清空vector a中的所有元素
```
```
默认构造函数，是在同时创建一个没有参数的构造函数，用于方便创建对象组
```
## 错误解决方案
错误：中文变成乱码
显示: converting to execution character set
解决方案:g++ main.cpp Tree_Node.cpp -finput-charset=GBK//将编码改为gbk
