# protBuf序列化后的数据的存储和读取
> 主要参考了Yngz_Miao大佬提供的开源项目：https://github.com/yngzMiao/protobuf-parser-tool
  
按照大佬提供的思路和代码自己实现了一下，并添加了注释和功能测试用例用做自己后续项目的组件，这里简单整理下使用的流程。

* 安装依赖：主要就是protobuf，本项目使用protobuf-3.19.4，可以之间按照这个教程安装：https://www.jianshu.com/p/aad9b0253cbe
* 环境：Ubuntu20.04
* protobuf的基本使用，参考教程：https://zhuanlan.zhihu.com/p/503759822
* 多个proto数据存储和读取的思路（Yngz_Miao大佬提供）：
* 1.使用4字节数据存储proto数据的字节数，小端大端都可以，只有后面解析的时候对应就行了，proto数据的字节可以通过protobuf提供的ByteSizeLong获得。
* 2.将这4字节数据存储于每个proto字节流的前面进行写入，即每个数据的长度在原来的基础上多了4个字节
* 3.读取的时候需要遍历整个文件，获得每条数据的偏移和长度，并在内存中存储为数组
* 4.根据数组下标获得这个数据的偏移和长度，然后去文件里读
  