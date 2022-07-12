### Cpp-Server-framework

---

#### 主要功能

从零开始造轮子，写一套cpp写的服务器框架，参考了java或者go中的一系列框架设计。

有些组件是照着开源项目敲的，有些是从零开始自己写的。

预期的功能模块和目前的进度：

- [x] 仿照log4j的日志系统 
  - 模块化输出程序运行日志
  - 自定义输出格式

- [ ] 线程模块（WIP）
  - 从零封装pthread 实现类似c++11的thread库
  - 锁的封装 线程安全的实现
  - 线程池
- [ ] 协程框架
  - c++20的新特性 本人也还不是特别清楚 只在go里面看到过一点 估计需要很久
- [x] 配置框架
  - 用类似spring框架的配置文件模式 
  - 用yaml配置文件来实现各个模块的配置
- [ ] socked I/O框架
  - 用于实现基础的网络服务器框架
- [ ] rpc模块
  - 从底层实现一套rpc调用接口

课余有时间闲着没事就会敲一点，也不知道多久能敲完，预计主要模块能在半年内写完。

---

#### 工具

IDEA Clion+vim

平时写写就用vim 由于不太会gdb 调试还是交给IDE吧

---

#### 目前项目结构

Src      
&nbsp; &nbsp; &nbsp; &nbsp; -- log 日志框架     
&nbsp; &nbsp; &nbsp; &nbsp; -- utils 工具类  

​               -- utils.h 工具类

​               -- singleton.h 单例实现接口

​               -- noncopyable.h 禁用类拷贝和复制的接口

​        -- Config 配置框架

​        -- thread 线程框架

Bin - 可以执行文件

tests - 测试
