# 基于HSV颜色特征的图像检索软件

Windows原生桌面程序，使用C++17和系统组件实现。

## 功能

- 恢复CIFAR-100二进制RGB图像
- 建立50000幅训练图像的72维HSV直方图库
- 按测试集编号或外部图像查询
- 选择直方图相交法或欧氏距离法
- 显示Top-10图像、类别与分值
- 导出查询图、候选BMP及CSV结果

## 编译与运行

准备Windows x64和MinGW-w64 C++工具链，在项目目录执行：

```powershell
.\build.ps1 -Compiler "C:\mingw64\bin\g++.exe"
.\ColorSearch.exe
```

也可将g++加入PATH后运行 `.\build.ps1`。工程另提供CMake配置。

## 操作

输入0至9999的测试集编号，选择相交法或欧氏距离法并检索；也可打开外部BMP、PNG、JPEG。点击“导出检索结果”，在“实验结果/界面导出”查看查询图、十幅候选图和Top10.csv。

## 数据集

使用[CIFAR-100官方数据集](https://www.cs.toronto.edu/~kriz/cifar.html)的binary version。数据集单独下载，解压后保持以下结构：

```text
数据集/cifar-100-binary/train.bin
数据集/cifar-100-binary/test.bin
数据集/cifar-100-binary/fine_label_names.txt
```

官方下载地址：https://www.cs.toronto.edu/~kriz/cifar-100-binary.tar.gz

压缩包MD5：`03b5dce01913d631647c71ecec9e9cb8`。程序按50000幅训练图像和10000幅测试图像读取。

## 文件

- src：窗口、处理算法及文件输入输出。
- build.ps1：Windows编译入口。
- CMakeLists.txt：CMake构建配置。

作者：柯长浩。仓库保留课程实验窗口标题。
