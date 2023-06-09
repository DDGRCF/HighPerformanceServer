# DDG High Performance Server

## 项目路径

```shell
bin -- 二进制
build -- 中间文件路径
CmakeLists.txt -- cmake的定义文件
lib -- 库的数据路径
Makefile -- task文件
ddg -- 源码文件
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

    输出的level由最底层决定

2. 组件之间太过冗余，可以减少冗余

## 协程库封装

1. c++封装的线程库，没有读写锁，在高并发里面读写锁能够提高并发速度
2. 线程id是top命令里面的线程号，输出的线程id和实际运行的线程id对应起来
3. 定义协程模块
4. shared this 问题 私有构造函数可能会被static函数调用
5. 主协程没有栈
6. 只能主协程调度协程

协程运作流程

1. new Scheduler
    * 如果use_caller，将调度器的主协程设为当前线程的创建新协程
    * 将调度器的主线程设为当前的线程
    * 将其设置rootFiber，将threads-1并将当前线程的id加入到线程id的列表中(也就是将当前主线程的非主协程设置为调度器的主协程)
    * 如果不使用，将rootFiber = -1
    * 设置线程的总数量

2. Scheduler start
    * 判断是否停止，已停止则不执行
    * 初始化线程池，并加入到线程，并把线程的运行目标转为Scheduler，最后添加线程id

3. Scheduler run
    * 设置当前运行的Scheduler，如果当前的线程不等于主线程，需要将调度器的主协程设置为当前线程的主协程
    * 然后循环运行查看是是否有回调任务或者携程
    * 如果任务里面的协程是回调也就是(thread == -1)或者协程指定的线程不为当前协程就跳过
    * 其次如果任务已经在执行了就跳过
    * 如果不上上面两种情况，那么就将移除任务或者是fiber
    * 然后根据移除的任务或者fiber选择特定的执行方式

4. Scheduler stop
    * TODO: 对于主线程中主协程的调用还是有问题，后面直接将主协程取消
    * 在stop中启动主线程调用，从而使主协程参与调度

## socket函数库

## http协议开发

## 分布协议

## 推荐系统
