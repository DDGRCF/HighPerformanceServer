# MySyLar

## 项目路径

```shell
bin -- 二进制
build -- 中间文件路径
CmakeLists.txt -- cmake的定义文件
lib -- 库的数据路径
Makefile
sylar -- 源码文件
tests -- 测试文件
```

## 日志系统

1. Log4J

    ```shell
    Logger (定义日志类别)
      |
      |------Formatter（日志格式）
      |
    Appender（日志输出地方）
    ```

## 协程库封装

1. c++封装的线程库，没有读写锁，在高并发里面读写锁能够提高并发速度
2. 线程id是top命令里面的线程号

## socket函数库

## http协议开发

## 分布协议

## 推荐系统
