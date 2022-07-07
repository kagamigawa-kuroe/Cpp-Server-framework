//
// Created by 王泓哲 on 06/07/2022.
//

#include "../src/config/config.h"
#include "../src/Log/log.h"
#include <iostream>
#include <yaml-cpp/yaml.h>

/// 类型的0 1 2 3 4 分别对应
/// Undefined, Null, Scaler, Sequence, Map
/// 未定义      空    纯量     数组      map

/// yaml文件中 -开头的为数组 内容包括到下一个-为止
/// : 前后对应一组Map

///在实例文件中 结构如下
/// logs 为根变量 是一个数组
/// 数组的第一个和第二个元素分别是一个map
/// 然后每个元素的map都有 name level appenders 三个key
/// name和level对应的value是纯良Scaler
/// appeders又是一个数组
/// 数组的第一个变量还是数组，有type和file两个类型
/// 数组的第二个变量也是数组 只有一个变量是type

/// 不难看出 总的来说 用法就是 由数组 存储map 在用map 存储纯量

void print_yaml(const YAML::Node& node, int level) {
    if(node.IsScalar()) {
        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << std::string(level * 4, ' ')
        << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if(node.IsNull()) {
        EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << std::string(level * 4, ' ')
        << "NULL - " << node.Type() << " - " << level;
    } else if(node.IsMap()) {
        for(auto it = node.begin();
        it != node.end(); ++it) {
            EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << std::string(level * 4, ' ')
            << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if(node.IsSequence()) {
        for(size_t i = 0; i < node.size(); ++i) {
            EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << std::string(level * 4, ' ')
            << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

euterpe::ConfigVar<int>::ptr int_value_1 = euterpe::Config::Lookup("systemport",(int)8080,"ststemport");
void base_test(){
    EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << int_value_1->getValue();
}

void yaml_read_test(){
    YAML::Node root = YAML::LoadFile("/Users/whz/learning/Cpp-Server-framework/Euterpe/bin/conf/log.yml");
    print_yaml(root,0);
}

euterpe::ConfigVar<int>::ptr port = euterpe::Config::Lookup("port",(int)8080,"port");
euterpe::ConfigVar<std::string>::ptr name = euterpe::Config::Lookup("name",std::string("tomcat"),"server");
void yaml_read_test_2(){
    std::cout<<"-----------------------------------------------------------"<<std::endl;
    EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << port->getValue();
    EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << name->toString();
    std::cout<<"-----------------------------------------------------------"<<std::endl;
}

int main(){
    yaml_read_test_2();
}