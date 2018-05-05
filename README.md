**Overview**

------

ID3算法例程。

**Obtaining source code**

```shell
git clone git@github.com:zegangYang/ID3_algorithm.git
```

**Build**

需要安装cmake。

```shell
cd ID3_Algorithm
cmake .
make
./ID3
```

**使用说明**

- 使用时打开*id3.h*，可以看到下面的宏定义：


```c
//开启weather代码
#define SAMPLE_WEATHER
//开启癌症例子代码
//#define SAMPLE_CANCER
```

上面的两个宏根据需求开启任意一个即可。

- 如果输出的决策树过于长，且有必要降低识别难度，需要降低数据集的数量，修改如下：

***main.cpp Line 11***

```c
//较少数据集
#define DATA_PATH "test.csv"
//较大数据集
#define DATA_PATH "cancer_data.csv"
```

**External links**

[ID3算法解释文章。](https://www.cise.ufl.edu/~ddd/cap6635/Fall-97/Short-papers/2.htm)

[ID3算法C语言实现。](http://id3alg.altervista.org/)



