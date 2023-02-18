# 改bug流程梳理

## 复现bug

- 按照bug出现的升级序列安装环境

- 按照bug情况描述复现bug

- 设置远程登录需要在网关开启

  ```
  https://10.0.40.71:60443/
  ```

  
  
  ```
  http://10.0.80.141/login//协同签名网关(SKSGW)按照这个地址登录
  ```
  
  ```
  - vi /cfg/GAD/core/site.d/default-port //查看监听的端口地址
  netstat -antlp | grep 80//查看监听的端口地址
  
  /etc/init.d/ssh  //这个注释掉(test -x /usr/sbin/sshd ||exit 0)下面那行
  /etc/ssh/sshd_config//SP7ssh配置文件
  ## 
  ```
  
  

## 修改bug

- 首先看SP4上有没有类似的问题

- 首先去查看SP4的gitlab上有没有类似的修改

- 看源码（这个有点难)

  ```
  print "<pre>";
  var_dump($name);
  print "</pre>";
  ctrl+alt+<-//使用鼠标+ctrl跳转到函数定义处，这样可以返回原来的位置
  console.log(currentIssuerKey);//js打印变量，在控制台上看
  本项目基本上都是当前页面提交，当前页面处理
  ```
- 一边解决问题一边把过程写成文档，为了梳理思路

  ```
  ## 问题详情
  
  ## 问题复现
  
  ## 问题追踪
  
  ## 问题解决
  
  ## 测试验证
  ```

  ```c
  <br>//markdown换行
  ```
  

## 上传合并

- 新建议题

- 在gitbash上面拉取项目，转到自己议题的那个分支，将原始版本的代码上传，如果已经有了原始代码，就不用上传了。

- ```shell
  git fetch origin #检查所有分支，如果有刚创建的分支会显示出来
  git branch -a #检查所有分支
  git checkout origin/1 #切换到自己创建的分支
  mkdir -p #递归创建目录
  git add ./* #将当前目录下的所有修改过的文件加入到暂存区
  git status #查看暂存区的状态
  git commit -m "test" #给提交的文件写注释
  git push origin newfetch #将文件上传到自己创建的那个分区
  ```

- 把自己修改后的代码进行上

  - 新建分支，在本地切换到该分支，查看自己修改的文件有没有在那个分支存在，如果存在就将该文件复制到old原始代码
  - 第一新建一个临时文件夹，一个文件夹为new存放修改后的代码，另一个文件夹为old存放原始代码
  - 修改完毕后将new文件夹里的文件取代虚拟机里的文件，进行测试，测试通过为止。
  - 先将old文件夹里面的文件存放到git对应路径上去，并push进入分支仓库。然后将new里面的文件替换掉git里面的文件，进行新的Push。

  ```
   git checkout main -- cert_param.php//用于将文件更新到最新main
  Ctrl+z 撤回; Ctrl+Shift+Z 反撤回
  注意换行时空格和Table的区别，如果前面的代码是Table那么后续就要Table，前面的代码时空格，后续就要空格，与前面代码保持一致，不然MR上去就不好看。
  ```




- 修改完成后把合并状态改为批准，将合并链接发给毅哥，玉喜哥，瑞明哥让他们看一下，后续不用管了

