////
//// Created by 王泓哲 on 06/07/2022.
////
//
//#include "../src/config/config.h"
//#include "../src/Log/log.h"
//#include <iostream>
//#include <yaml-cpp/yaml.h>
//#include <vector>
//#include <list>
//#include <set>
//
///// 类型的0 1 2 3 4 分别对应
///// Undefined, Null, Scaler, Sequence, Map
///// 未定义      空    纯量     数组      map
//
///// yaml文件中 -开头的为数组 内容包括到下一个-为止
///// : 前后对应一组Map
//
/////在实例文件中 结构如下
///// logs 为根变量 是一个数组
///// 数组的第一个和第二个元素分别是一个map
///// 然后每个元素的map都有 name level appenders 三个key
///// name和level对应的value是纯量scaler–
///// appeders又是一个数组
///// 数组的第一个变量还是数组，有type和file两个类型
///// 数组的第二个变量也是数组 只有一个变量是type
//
///// 不难看出 总的来说 用法就是 由数组 存储map 在用map 存储纯量
//
//class Person {
//public:
//    Person() {};
//    std::string m_name;
//    int m_age = 0;
//    bool m_sex = 0;
//
//    std::string toString() const {
//        std::stringstream ss;
//        ss << "[Person name=" << m_name
//        << " age=" << m_age
//        << " sex=" << m_sex
//        << "]";
//        return ss.str();
//    }
//
//    bool operator==(const Person& oth) const {
//        return m_name == oth.m_name
//        && m_age == oth.m_age
//        && m_sex == oth.m_sex;
//    }
//};
//
//namespace euterpe {
//
//    template<>
//    class LexicalCast<std::string, Person> {
//    public:
//        Person operator()(const std::string& v) {
//            YAML::Node node = YAML::Load(v);
//            Person p;
//            p.m_name = node["name"].as<std::string>();
//            p.m_age = node["age"].as<int>();
//            p.m_sex = node["sex"].as<bool>();
//            return p;
//        }
//    };
//
//    template<>
//    class LexicalCast<Person, std::string> {
//    public:
//        std::string operator()(const Person& p) {
//            YAML::Node node;
//            node["name"] = p.m_name;
//            node["age"] = p.m_age;
//            node["sex"] = p.m_sex;
//            std::stringstream ss;
//            ss << node;
//            return ss.str();
//        }
//    };
//}
//    void print_yaml(const YAML::Node& node, int level) {
//        if(node.IsScalar()) {
//            EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << std::string(level * 4, ' ')
//            << node.Scalar() << " - " << node.Type() << " - " << level;
//        } else if(node.IsNull()) {
//            EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << std::string(level * 4, ' ')
//            << "NULL - " << node.Type() << " - " << level;
//        } else if(node.IsMap()) {
//            for(auto it = node.begin();
//            it != node.end(); ++it) {
//                EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << std::string(level * 4, ' ')
//                << it->first << " - " << it->second.Type() << " - " << level;
//                print_yaml(it->second, level + 1);
//            }
//        } else if(node.IsSequence()) {
//            for(size_t i = 0; i < node.size(); ++i) {
//                EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << std::string(level * 4, ' ')
//                << i << " - " << node[i].Type() << " - " << level;
//                print_yaml(node[i], level + 1);
//            }
//        }
//    }
//
//// //   void base_test(){
////        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << int_value_1->getValue();
////    }
//
//    void yaml_read_test(){
//        // YAML::Node root = YAML::LoadFile("/Users/whz/learning/Cpp-Server-framework/Euterpe/bin/conf/log.yml");
//        // print_yaml(root,0);
//    }
//
//    // euterpe::ConfigVar<int>::ptr port = euterpe::config::Lookup("port",(int)8080,"port");
//    // euterpe::ConfigVar<std::string>::ptr name = euterpe::config::Lookup("name",std::string("tomcat"),"server");
//    void yaml_read_test_2(){
//        std::vector<int> temp = {1,2,3};
//        std::list<int> t2 = {1,2,3};
//
//        euterpe::ConfigVar<int>::ptr port = euterpe::Config::Lookup("system.port",(int)8080,"port");
//        euterpe::ConfigVar<std::vector<int>>::ptr v1 = euterpe::Config::Lookup("system.v1",temp,"v1");
//        euterpe::ConfigVar<std::list<int>>::ptr l1 = euterpe::Config::Lookup("system.l1",t2,"v1");
//        std::cout<<"-----------------------------------------------------------"<<std::endl;
//
//        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << port->getValue();
//        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << v1->toString();
//        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << l1->toString();
//        YAML::Node root1 = YAML::LoadFile("/Users/whz/learning/Cpp-Server-framework/Euterpe/bin/conf/log.yml");
//        euterpe::Config::LoadFromYaml(root1);
//        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << port->getValue();
//        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << v1->toString();
//        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << l1->toString();
//        std::cout<<"-----------------------------------------------------------"<<std::endl;
//    }
//
//    void test_person(){
//        euterpe::ConfigVar<Person>::ptr g_person = euterpe::Config::Lookup("class.person", Person(), "system person");
//        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
//        YAML::Node root = YAML::LoadFile("/Users/whz/learning/Cpp-Server-framework/Euterpe/bin/conf/log.yml");
//        euterpe::Config::LoadFromYaml(root);
//        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
//    }
//
//    void test(){
//         YAML::Node root = YAML::LoadFile("/Users/whz/learning/Cpp-Server-framework/Euterpe/bin/conf/log.yml");
//         euterpe::Config::LoadFromYaml(root);
//         // std::set<euterpe::LogDefine> temp = {};
//         // euterpe::ConfigVar<std::set<euterpe::LogDefine>>::ptr a = euterpe::config::Lookup("logs", temp, "system person");
//         // EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << ((a->getValue()).begin()->appenders)[0].file << " - " << a->toString();
//         auto l = euterpe::LoggerMgr::GetInstance()->getLogger("root");
//         auto l2 = euterpe::LoggerMgr::GetInstance()->getLogger("system");
//         EUTERPE_LOG_INFO(l) << "loggernot find";
//    }
////    int main(){
////        test();
////    }
