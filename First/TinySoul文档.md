# TinySoul文档
## 需求
用于扫描文件夹，其具体要求如下

![image-20211106143846829](C:\Users\admin\AppData\Roaming\Typora\typora-user-images\image-20211106143846829.png)

## 设计

分成几个模块

初始化模块：

- 检测依赖是否缺少依赖，缺少则视为异常，退出工具

* 加载抑制列表

交互模块：

- 根据帮助页面提供的模板指定目录和初始markdown文件。

操作流程

```shell
# 输入参数
# $1 markdown文件名
# $2 检索目录
# $3 抑制列表文档
# $4 输出文档名
# eg
sh tinysoul Makedown.md /usr/hello/hello test.conf out.txt
```

实现流程

初始化，检测是否缺少依赖工具

读取输入的参数

然后检索指定目录下的所有文件，列出表格

然后检索markdown文档中引用的所有文档，列出表格

然后检索markdown文档中引用的所有文档里面引用不当的文档，列出表格。

然后检索指定目录下没有被markdown文档引用的文档，列出表格。

拓展：

如果需要检测指定网页的文件

先把指定网址的文件下载下来，然后再检测。

```shell
wget $old -O /opt/patch/nsag-old.zip >/dev/null
  if [ $? != 0 ]; then
    echo "下载文件1失败！"
    exit 1
  fi
```

