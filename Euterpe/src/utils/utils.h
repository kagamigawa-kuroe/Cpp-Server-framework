//
// Created by 王泓哲 on 04/07/2022.
//

#ifndef EUTERPE_UTILS_H
#define EUTERPE_UTILS_H

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <cstdlib>
#include <typeinfo>
#include <cxxabi.h>
#include <cstdint>
#include <string>
namespace euterpe{
    pid_t GetThreadId();

    uint32_t GetFiberId();

    std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

    /// 可以返回完整的类名
    /// 直接用typeid().name() 当类名过长时无法返回正确类名
    template<class T>
    const char* TypeToName() {
        static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
        return s_name;
    }

    /**
 * @brief 获取当前时间的毫秒
 */
    uint64_t GetCurrentMS();

/**
 * @brief 获取当前时间的微秒
 */
    uint64_t GetCurrentUS();
}

#endif //EUTERPE_UTILS_H
