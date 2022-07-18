//
// Created by 王泓哲 on 04/07/2022.
//

#include "utils.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <dirent.h>
#include <execinfo.h>
#include "../Log/log.h"

namespace euterpe{

    static euterpe::Logger::ptr g_logger = EUTERPE_LOG_NAME("system");

    pid_t GetThreadId(){
        /// 只在linux环境下可用
        pid_t tid = syscall(SYS_gettid);

        /// 由于测试环境在mac 使用pthread替代
        /// uint64_t tid;
        /// pthread_threadid_np(NULL, &tid);
        return tid;
    };
    uint32_t GetFiberId(){
        return 0;
    };

    /// 获取函数的原型
    static std::string demangle(const char* str) {
        size_t size = 0;
        int status = 0;
        std::string rt;
        rt.resize(256,' ');
        if(1 == sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0])) {
            char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
            if(v) {
                std::string result(v);
                free(v);
                return result;
            }
        }
        if(1 == sscanf(str, "%255s", &rt[0])) {
            int i = 0;
            while(rt[i]!=' '){
                ++i;
            }
            --i;
            rt = rt.substr(0,i);
            return rt;
        }
        return str;
    }

    void Backtrace(std::vector<std::string>& bt, int size, int skip) {
        void** array = (void**)malloc((sizeof(void*) * size));

        /// 全局作用域符号，双冒号要放在开头
        size_t s = ::backtrace(array, size);

        char** strings = backtrace_symbols(array, s);
        if(strings == NULL) {
            EUTERPE_LOG_ERROR(g_logger) << "backtrace_synbols error";
            return;
        }

        for(size_t i = skip; i < s; ++i) {
            bt.push_back(demangle(strings[i]));
        }

        free(strings);
        free(array);
    }

    std::string BacktraceToString(int size, int skip, const std::string& prefix) {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;
        for(size_t i = 0; i < bt.size(); ++i) {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }
}