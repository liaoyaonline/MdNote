# 上传必备

## 常用流程

```
git branch -av //查看当前的分支
git branch develop//创建develop分支
git checkout develop//切换到develop分支
git add filename//将修改后的文件加入暂存区
git status//查看暂存区状态
git commit -m "tag"//增加注释
git push origin develop//将文件上传到远程的develop分支，后续到git项目页将develop分支和master分支合并。

```

## 常用标签

```
[ADD] :新加文件和功能
[FIX]：修补bug
[REFACTOR]：重构(即不是新增功能，也不是修改bug的代码变动)

```



# 文件介绍

Working_tree:工作区，用于存储草稿，灵感，一些零碎的文档

Index:暂存区，用于将工作区的一些文档进行加工发布

OutPut_tree:用于保存已经发表的一些文档，比如CSDN上