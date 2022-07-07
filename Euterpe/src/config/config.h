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

    //配置变量 T类型
    template<class T>
    class ConfigVar : public ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVar> ptr;
        ConfigVar(const std::string &name, const T &default_value, const std::string &description = "")
                : ConfigVarBase(name, description), m_val(default_value) {};

        std::string toString() override {
            try {
                return boost::lexical_cast<std::string>(m_val);
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
                m_val = boost::lexical_cast<T>(val);
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
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
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
