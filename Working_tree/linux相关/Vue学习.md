# vue学习指南(Vue2　实践揭秘)
# 第1.３节有两个问题。
第一个，样式绑定的时候webpack.config.js的modules设置内容
原文为
```
      {
        test: /\.less$/,
        loader: "style!css!less"
      }
```
现在已经不支持缩写了，应当修改为
```
      {
        test: /\.less$/,
        loader: "style-loader!css-loader!less-loader"
      }
```
第二个，编译后仍然提示this.getOptions is not a function
主要是less-loader安装版本过高引起的。
解决方案：
```
npm uninstall less-loader
npm install less-loader@5.0.0
```