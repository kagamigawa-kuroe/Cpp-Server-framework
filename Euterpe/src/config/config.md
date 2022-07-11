### config模块

---
#### 基础用法
1. 加载一个yaml文件   
   YAML::Node root = YAML::LoadFile(filename);     

2. 为想要使用的变量添加默认值  
   **euterpe::ConfigVar<std::set<LogDefine> >::ptr g_log_defines =
   euterpe::Config::Lookup("logs", std::set<LogDefine>(), "logs config");**  
   只有在加入过默认值后，才能正常使用 这里的Lookup函数
   会去Config类中维护的ConfigVar类型的的Map中查找你要的变量
   找不到就新建一个 然后设置起为默认值

3. 调用euterpe::Config::LoadFromYaml方法   
   将所有的YAML文件中的类载入到Config类中
   对应了yaml文件中的key和value
   前提是你一定要调用Lookup方法yaml中的变量写入过Config中

4. 在LoadFromYaml载入的过程中 他会将yaml中的node格式转换成字节码
   然后对字节码调用fromstring函数 也就是我们模版特化过的LexicalCast系列函数
   根据具体的m_var类型 将字节码转化成变量
   然后进行替换

5. 然后 你就可以从Config的map中根据名字找到yaml文件中的变量了

总结一下config系统的使用方法：  
首先，用euterpe::Config::Lookup声明一个变量的类型和默认值   
然后，加载yaml文件 更新这个变量    
最后，当你再使用这个变量的时候 值就已经是yaml文件中的值了    

要注意 一定要先声明 再加载yaml文件 直接加载是不行的


---
#### config与log是如何整合的

其实也很简单，我们为每一个Configvar类添加了一个新的vector 记录一系列回调函数  
这些回调函数 会在setValue函数被调用时触发，也就是上述第4步的过程中 调用fromstr的时候  
我们在main函数开始前 为其添加一个静态全局变量  
在这个全局变量的构造函数中 为g_log_defines 这个变量在config.cpp被声明 就像上述说的那样   
也就是记录了所有logs参数的一个ConfigVar类型的全局变量  
为他加入一个回调函数  
这个回调函数的功能是：比较原始value和现在value的值 并对logmanager类中管理的logger进行更新  
这样一来，每当我们使用LoadFile函数时，logmanager也会被相应地更新  

综上 在整合完陪配置系统后 日记系统的用法就是

1. 在yaml文件中写入配置信息 以logs开头
2. LoadFile 然后 LoadFromYaml
3. 使用euterpe::LoggerMgr::GetInstance()->getLogger(loggername) 获取logger
4. 使用EUTERPE_LOG_INFO(your_logger) << "info"; 进行输出
