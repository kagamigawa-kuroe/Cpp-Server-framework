//
// Created by 王泓哲 on 06/07/2022.
//

#ifndef EUTERPE_CONFIG_H
#define EUTERPE_CONFIG_H

#include <memory>
#include <string>
#include <sstream>
#include <unordered_map>
/// boost中的一个类型转换库 可以把string变成int
#include <boost/lexical_cast.hpp>
#include "../Log/log.h"
#include "../utils/utils.h"
#include <yaml-cpp/yaml.h>
#include <unordered_set>

namespace euterpe {

    // 配置变量的基类
    class ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;

        ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description) {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
        /// OutputIt transform( InputIt first1, InputIt last1, OutputIt d_first,
        ///                     UnaryOperation unary_op );
        /// transform函数 从第一个参数开始 到第二个参数结束 统一进行第四个参数的操作 放入第三个参数中

        virtual ~ConfigVarBase() {};

        const std::string &getName() const { return m_name; }

        const std::string &getDescription() const { return m_description; }

        virtual std::string toString() = 0;

        virtual bool fromString(const std::string &val) = 0;

        virtual std::string getTypeName() const = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };

    /// 基础类型的转换
    template<class F, class T>
    class LexicalCast {
    public:
        /// 重载()运算符
        /// 用法 例如一个LexicalCast的对象A
        /// 就可以直接A(V)
        /// 就会进入这个函数
        T operator()(const F &v) {
            return boost::lexical_cast<T>(v);
        }
    };

    /// 除了基础类意外的所有其他类的模版特化
    template<class T>
            class LexicalCast<std::string, std::vector<T> > {
            public:
                std::vector<T> operator()(const std::string& v) {
                    YAML::Node node = YAML::Load(v);
                    typename std::vector<T> vec;
                    std::stringstream ss;
                    for(size_t i = 0; i < node.size(); ++i) {
                        ss.str("");
                        ss << node[i];
                        vec.push_back(LexicalCast<std::string, T>()(ss.str()));
                    }
                    return vec;
                }
            };

    template<class T>
            class LexicalCast<std::vector<T>, std::string> {
            public:
                std::string operator()(const std::vector<T>& v) {
                    YAML::Node node(YAML::NodeType::Sequence);
                    for(auto& i : v) {
                        node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
                    }
                    std::stringstream ss;
                    ss << node;
                    return ss.str();
                }
            };

    template<class T>
            class LexicalCast<std::string, std::list<T> > {
            public:
                std::list<T> operator()(const std::string& v) {
                    YAML::Node node = YAML::Load(v);
                    typename std::list<T> vec;
                    std::stringstream ss;
                    for(size_t i = 0; i < node.size(); ++i) {
                        ss.str("");
                        ss << node[i];
                        vec.push_back(LexicalCast<std::string, T>()(ss.str()));
                    }
                    return vec;
                }
            };

    template<class T>
            class LexicalCast<std::list<T>, std::string> {
            public:
                std::string operator()(const std::list<T>& v) {
                    YAML::Node node(YAML::NodeType::Sequence);
                    for(auto& i : v) {
                        node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
                    }
                    std::stringstream ss;
                    ss << node;
                    return ss.str();
                }
            };

    template<class T>
            class LexicalCast<std::string, std::set<T> > {
            public:
                std::set<T> operator()(const std::string& v) {
                    YAML::Node node = YAML::Load(v);
                    typename std::set<T> vec;
                    std::stringstream ss;
                    for(size_t i = 0; i < node.size(); ++i) {
                        ss.str("");
                        ss << node[i];
                        vec.insert(LexicalCast<std::string, T>()(ss.str()));
                    }
                    return vec;
                }
            };

    template<class T>
            class LexicalCast<std::set<T>, std::string> {
            public:
                std::string operator()(const std::set<T>& v) {
                    YAML::Node node(YAML::NodeType::Sequence);
                    for(auto& i : v) {
                        node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
                    }
                    std::stringstream ss;
                    ss << node;
                    return ss.str();
                }
            };

    template<class T>
            class LexicalCast<std::string, std::unordered_set<T>> {
            public:
                std::unordered_set<T> operator()(const std::string& v) {
                    YAML::Node node = YAML::Load(v);
                    typename std::unordered_set<T> vec;
                    std::stringstream ss;
                    for(size_t i = 0; i < node.size(); ++i) {
                        ss.str("");
                        ss << node[i];
                        vec.insert(LexicalCast<std::string, T>()(ss.str()));
                    }
                    return vec;
                }
            };

    template<class T>
            class LexicalCast<std::unordered_set<T>, std::string> {
            public:
                std::string operator()(const std::unordered_set<T>& v) {
                    YAML::Node node(YAML::NodeType::Sequence);
                    for(auto& i : v) {
                        node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
                    }
                    std::stringstream ss;
                    ss << node;
                    return ss.str();
                }
            };

    template<class T>
            class LexicalCast<std::string, std::map<std::string, T> > {
            public:
                std::map<std::string, T> operator()(const std::string& v) {
                    YAML::Node node = YAML::Load(v);
                    typename std::map<std::string, T> vec;
                    std::stringstream ss;
                    for(auto it = node.begin();
                    it != node.end(); ++it) {
                        ss.str("");
                        ss << it->second;
                        vec.insert(std::make_pair(it->first.Scalar(),
                                                  LexicalCast<std::string, T>()(ss.str())));
                    }
                    return vec;
                }
            };

    template<class T>
            class LexicalCast<std::map<std::string, T>, std::string> {
            public:
                std::string operator()(const std::map<std::string, T>& v) {
                    YAML::Node node(YAML::NodeType::Map);
                    for(auto& i : v) {
                        node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
                    }
                    std::stringstream ss;
                    ss << node;
                    return ss.str();
                }
            };

    template<class T>
            class LexicalCast<std::string, std::unordered_map<std::string, T> > {
            public:
                std::unordered_map<std::string, T> operator()(const std::string& v) {
                    YAML::Node node = YAML::Load(v);
                    typename std::unordered_map<std::string, T> vec;
                    std::stringstream ss;
                    for(auto it = node.begin();
                    it != node.end(); ++it) {
                        ss.str("");
                        ss << it->second;
                        vec.insert(std::make_pair(it->first.Scalar(),
                                                  LexicalCast<std::string, T>()(ss.str())));
                    }
                    return vec;
                }
            };

    template<class T>
            class LexicalCast<std::unordered_map<std::string, T>, std::string> {
            public:
                std::string operator()(const std::unordered_map<std::string, T>& v) {
                    YAML::Node node(YAML::NodeType::Map);
                    for(auto& i : v) {
                        node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
                    }
                    std::stringstream ss;
                    ss << node;
                    return ss.str();
                }
            };

    /// 配置变量 T类型
    /// boost中的LexicalCast只支持简单类型tostring
    /// 如map vector set list这种 都没法tostring
    /// 为了让任意yaml文件中的类型都能进行输出
    /// 我们直接提供两个类 Fromstr和tostr
    /// 在构造ConfigVar变量的时候就传入这两个类
    /// 可以针对不同的T类型 有不同的输出
    template<class T,class FromStr = LexicalCast<std::string, T>
            ,class ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVar> ptr;
        ConfigVar(const std::string &name, const T &default_value, const std::string &description = "")
                : ConfigVarBase(name, description), m_val(default_value) {};

        std::string toString() override {
            try {
                return ToStr()(m_val);;
            } catch (std::exception &e) {
                EUTERPE_LOG_ERROR(EUTERPE_LOG_ROOT()) << "ConfigVar::tostring exception"
                                                      << e.what() << "convert from " << typeid(m_val).name()
                                                      << "to string";
                /// type_info对象是c++获取对象类型的一种办法 类似于反射
                /// typeid.name() 获取类型
                /// 程序中创建type_info对象的唯一方法是使用typeid操作符
            }
            return "";
        }

        bool fromString(const std::string &val) override {
            try {
                setValue(FromStr()(val));
            } catch (std::exception &e) {
                EUTERPE_LOG_ERROR(EUTERPE_LOG_ROOT()) << "ConfigVar::tostring exception"
                                                      << e.what() << "convert from string to" << typeid(m_val).name();
            }
            return false;
        }

        T getValue() { return m_val; }

        void setValue(T mVal) { m_val = mVal; }

        std::string getTypeName() const override { return TypeToName<T>(); }

    private:
        T m_val;
    };

    class Config {
    public:
        /// 存储的是yaml文件中读取的每一个 变量名 和 对应的变量
        typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

        /// 用参数名查询一个参数
        /// 如果存在 直接返回 不存在 即用传入的参数创建
        /// 如果开头字母不对 爆出异常
        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name,
                                                 const T &default_value, const std::string &description = "") {
            auto it = GetDatas().find(name);
            if (it != GetDatas().end()) {
                /// 把一个对象转换成该对象类型的只能指针
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                if (tmp) {
                    EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << "Lookup name=" << name << " exists";
                    return tmp;
                } else {
                    EUTERPE_LOG_ERROR(EUTERPE_LOG_ROOT()) << "Lookup name=" << name << " exists but type not "
                                                          << TypeToName<T>() << " real_type="
                                                          << it->second->getTypeName()
                                                          << " " << it->second->toString();
                    return nullptr;
                }
            }

            if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")!= std::string::npos) {
                EUTERPE_LOG_ERROR(EUTERPE_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }

            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            GetDatas()[name] = v;
            return v;
        }

        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name) {
            auto it = GetDatas().find(name);
            if (it == GetDatas().end()) {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
        }

        static void LoadFromYaml(const YAML::Node& root);

        static ConfigVarBase::ptr LookupBase(const std::string& name);

        static void Visit(std::function<void(ConfigVarBase::ptr)> cb);

        static void LoadFromConfDir(const std::string& path, bool force = false);

    private:
        /// 在static方法中 用了一个static变量
        /// 这样 对类来说 不论是这个变量还是获取这个变量的接口 都是共享的
        static ConfigVarMap &GetDatas() {
            static ConfigVarMap s_datas;
            return s_datas;
        }
    };
}

#endif //EUTERPE_CONFIG_H
