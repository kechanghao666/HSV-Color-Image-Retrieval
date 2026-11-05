## 数据集

使用[CIFAR-100官方数据集](https://www.cs.toronto.edu/~kriz/cifar.html)的binary version。数据集单独下载，解压后保持以下结构：

```text
数据集/cifar-100-binary/train.bin
数据集/cifar-100-binary/test.bin
数据集/cifar-100-binary/fine_label_names.txt
```

官方下载地址：https://www.cs.toronto.edu/~kriz/cifar-100-binary.tar.gz

压缩包MD5：`03b5dce01913d631647c71ecec9e9cb8`。程序按50000幅训练图像和10000幅测试图像读取。
