# Python中的备忘语法
- def //标识，告诉后面是个函数定义
- __init__//初始化函数

- self//实例本身

- n:m//从n列到m列

- n:m:2//从n列到m列每次加2

- np.concatenate((a,b),axis = 1)//将两个队列加到一起以列，如果axis= 0就是以行

- np.random.shuffle(a)//打乱a的数据排序

- sklearn.datasets.make_moons(n_samples=100, shuffle=True, noise=None, random_state=None)//n_sample生成样本数量,shuffle是否打乱,noise是否加入高斯噪声,random_state生成随机种子，给定int数据保证每次生成数据一样

- a = [1,2,3] b = [4,5,6] zip(a,b) = [(1,4),(2,5),(3,6)]//打包成元组的列表

- x = np.array([1,2],[3,4]]) x.ravel() = （[1,2,3,4])//降维

- x = plot_2d_separator(forest, X_train, fill=True, ax=axes[-1, -1],alpha=0.4)//对图进行着色

- mglearn.discrete_scatter(X_train[:, 0], X_train[:, 1], y_train)//绘制散点图