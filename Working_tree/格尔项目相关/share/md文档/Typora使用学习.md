# Typora使用学习

## 常用

- 流程图

  ![img](https://pic2.zhimg.com/80/v2-de8ab04af128f209b014d2d1b51ba5b5_720w.jpg)

- 流程图

  

对于Flowchart，可以使用下面的code，然后语法选择flow

```text
st=>start: 闹钟响起
op=>operation: 与床板分离
cond=>condition: 分离成功?
e=>end: 快乐的一天

st->op->cond
cond(yes)->e
cond(no)->op
```

![img](https://pic1.zhimg.com/80/v2-9cad272c65e513caa60c454fc72cb8d0_720w.jpg)

除了Sequence和Flowchart两类图之外，Mermaid还提供一种叫做gantt的图。插入下面的代码，然后语法选`mermaid`就会自动渲染成gantt图了。

```text
gantt
        dateFormat  YYYY-MM-DD
        title 快乐的生活
        section 吃一把鸡就学习
        学习            :done,    des1, 2014-01-06,2014-01-09
        疯狂学习               :active,  des2, 2014-01-09, 3d
        继续疯狂学习               :         des3, after des2, 5d
        吃鸡！               :         des4, after des3, 4d
        section 具体内容
        学习Python :crit, done, 2014-01-06,72h
        学习C++          :crit, done, after des1, 2d
        学习Lisp             :crit, active, 3d
        学习图形学        :crit, 4d
        跳伞           :2d
        打枪                      :2d
```

![img](https://pic4.zhimg.com/80/v2-c23dca694b9f8d54b88ebefde5fc825b_720w.jpg)

## **表格**

Typora中的表格是完全图形化操作的，非常方便。之前用过一些App（比如emacs），可以用文字+快捷键的方式编辑表格，然而需要记很多东西，不经常用就忘了。Typora的这个是真好用。表格的快捷键也很好记，`cmd + T` (Table)。也可以很方便地编辑表格的形状，如下图

![img](https://pic1.zhimg.com/80/v2-1ef735ed6a7b486201002508567c9b28_720w.jpg)

