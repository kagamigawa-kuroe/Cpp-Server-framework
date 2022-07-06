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

namespace euterpe{
    pid_t GetThreadId();

    uint32_t GetFiberId();

    /// 可以返回完整的类名
    /// 直接用typeid().name() 当类名过长时无法返回正确类名
    template<class T>
    const char* TypeToName() {
        static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
        return s_name;
    }
}

#endif //EUTERPE_UTILS_H
