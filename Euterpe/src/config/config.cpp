//
// Created by 王泓哲 on 06/07/2022.
//

#include "config.h"
#include <yaml-cpp/yaml.h>
#include <functional>

/// 首先 为了储存一个个配置文件参数 我们有一个类叫做 ConfigVar 封装了一个个参数变量
/// 还有一个叫config的类，维护了一个map，包含了所有的ConfigVar对象
/// 这些ConfigVar对象有tostring 和 fromstring方法
/// 用来将其变成string 和 通过string变成他们
/// tostring 和 fromstring 通过模版特化 实现了vector，map等数据类型的这两种方法

/// 当我们想要读取yaml配置文件中的参数到上述config类的map中时
/// 我们会调用LoadFromConfDir方法
/// LoadFromConfDir函数 调用 LoadFromYaml函数
/// LoadFromYaml函数 先调用 ListAllMember函数
/// 将YAML文件中的所有变量取出
/// LoadFromYaml函数 然后调用 LookupBase 去变量map中查询是否含有这个名字的变量
/// 如果找到了
/// LoadFromYaml函数 调用fromString方法 设置这个ConfigVar的值

/// 所以 当你想用yaml文件去加载一个变量ConfigVar 的时候
/// 要先把它写入config类中才行
/// 使用Lookup函数创建一个ConfigVar到config类中 并且赋予默认值
/// 然后再读取yaml配置文件

namespace euterpe{

    static euterpe::Logger::ptr g_logger = EUTERPE_LOG_NAME("system");

    static std::map<std::string, uint64_t> s_file2modifytime;
    static euterpe::Mutex s_mutex;

    ConfigVarBase::ptr  Config::LookupBase(const std::string& name){
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        return it == GetDatas().end() ? nullptr : it->second;
    };

    static void ListAllMember(const std::string& prefix,
                              const YAML::Node& node,
                              std::list<std::pair<std::string, const YAML::Node> >& output) {
        if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")
        != std::string::npos) {
            EUTERPE_LOG_ERROR(g_logger) << "config invalid name: " << prefix << " : " << node;
            return;
        }
        output.push_back(std::make_pair(prefix, node));
        if(node.IsMap()) {
            for(auto it = node.begin();
            it != node.end(); ++it) {
                ListAllMember(prefix.empty() ? it->first.Scalar()
                : prefix + "." + it->first.Scalar(), it->second, output);
            }
        }
    }

    /// 会讲yaml文件中的所有对象都封装到config类中
    void Config::LoadFromYaml(const YAML::Node& root) {
        std::list<std::pair<std::string, const YAML::Node> > all_nodes;
        ListAllMember("", root, all_nodes);

        for(auto& i : all_nodes) {
            std::string key = i.first;
            if(key.empty()) {
                continue;
            }

            std::transform(key.begin(), key.end(), key.begin(), ::tolower);

            /// lookupbase 在发现没有的时候会新建
            ConfigVarBase::ptr var = LookupBase(key);

            /// 从yaml文件中读取的流程为
            /// 从yaml文件中读到字符串
            /// 然后调用fromstring函数 在调用fromstr转换类
            /// 根据var中模版T的类型 将这个str转成你想要的类型
            /// 不同类型都被模版特化了
            if(var) {
                if(i.second.IsScalar()) {
                    var->fromString(i.second.Scalar());
                } else {
                    std::stringstream ss;
                    ss << i.second;
                    var->fromString(ss.str());
                }
            }
        }
    }

    void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
        RWMutexType::ReadLock lock(GetMutex());
        ConfigVarMap& m = GetDatas();
        for(auto it = m.begin();
        it != m.end(); ++it) {
            cb(it->second);
        }

    }

}