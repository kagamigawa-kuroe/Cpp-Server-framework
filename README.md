### Cpp-Server-framework

---

#### 主要功能

从零开始造轮子，写一套cpp写的服务器框架，参考了java或者go中的一系列框架设计。

有些组件是照着开源项目敲的，有些是从零开始自己写的。

预期的功能模块和目前的进度：

- [x] 仿照log4j的日志系统 
  - 模块化输出程序运行日志
  - 自定义输出格式
- [x] 线程模块
  - 从零封装pthread 实现类似c++11的thread库
  - 锁的封装 线程安全的实现
  - 线程池( 基于协程任务 )
- [x] 协程框架
  - ~~c++20的新特性 本人也还不是特别清楚 只在go里面看到过一点 估计需要很久~~
  - 在对比了几个主流框架后 觉得c++20的协程api不是很方便  ~~真不是人写的啊c++~~ 于是打算从底层开始写一个
  - 采用linux原生ucontext.h库 采用上下文切换的形式 封装了一套协程框架
  - 以及基于线程池的协程调度的相关支持
- [x] 配置框架
  - 用类似spring框架的配置文件模式 
  - 用yaml配置文件来实现各个模块的配置
- [x] I/O框架
  - 继承协程调度框架 整合epoll实现socket连接的任务调度
- [x] 定时器模块
  - 实现一套定时任务管理系统，可以简单的删除添加定时任务
  - 与IO调度模块相结合 实现更高级的IO事件触发以及执行
- [x] hook钩子模块
  - 主要用于对原生的库函数进行一系列的拓展 定制更细致化的功能
  - 该hook是线程级别的 每个线程都有启动hook单独的开关
  - 重写系统读写函数 通过定时器和epoll实现异步处理 减少阻塞消耗的时间 从而提高性能
- [ ] socket封装(WIP)
  - 基于socket原生api封装的类，实现更简单的socket通信
  - 实现tcp模块的封装
  - 实现http协议的解析 以及http服务器快速搭建的框架
  - 实现类似servlet的url定向功能
- [ ] rpc模块
  - 从底层实现一套rpc调用接口
- [ ] mysql/redis的驱动 以及zookeeper调用的封装？(看心情)

课余有时间闲着没事就会敲一点，也不知道多久能敲完，预计主要模块能在半年内写完。

---

#### 工具

IDEA Clion+vim

平时写写就用vim 由于不太会gdb 调试还是交给IDE吧

---

#### 目前项目结构

./Euterpe/src    
├── config   
│   ├── config.cpp   
│   ├── config.h   
│   └── config.md   
├── coroutines   
│   ├── fiber.h   
│   └── fuber.cpp   
├── euterpe.h   
├── image   
│   └── Log_usgae.jpg   
├── Log   
│   ├── log.cpp   
│   ├── log.h   
│   └── Log_note.md   
├── thread   
│   ├── euterpe_thread.cpp   
│   ├── euterpe_thread.h   
│   ├── mutex.cpp   
│   ├── mutex.h   
│   ├── Thread.md  
│   └── 锁的封装.md   
└── utils   
    ├── macro.h   
    ├── noncopyable.h   
    ├── singleton.h   
    ├── utils.cpp   
    └── utils.h    

