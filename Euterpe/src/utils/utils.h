//
// Created by 王泓哲 on 04/07/2022.
//

#ifndef EUTERPE_UTILS_H
#define EUTERPE_UTILS_H
#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <cstdlib>
#include <typeinfo>
#include <cxxabi.h>
#include <cstdint>
#include <string>
#include <vector>
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

    class FSUtil {
    public:
        static void ListAllFile(std::vector<std::string>& files
                ,const std::string& path
                ,const std::string& subfix);
        static bool Mkdir(const std::string& dirname);
        static bool IsRunningPidfile(const std::string& pidfile);
        static bool Rm(const std::string& path);
        static bool Mv(const std::string& from, const std::string& to);
        static bool Realpath(const std::string& path, std::string& rpath);
        static bool Symlink(const std::string& frm, const std::string& to);
        static bool Unlink(const std::string& filename, bool exist = false);
        static std::string Dirname(const std::string& filename);
        static std::string Basename(const std::string& filename);
        static bool OpenForRead(std::ifstream& ifs, const std::string& filename
                ,std::ios_base::openmode mode);
        static bool OpenForWrite(std::ofstream& ofs, const std::string& filename
                ,std::ios_base::openmode mode);
    };
}

#endif //EUTERPE_UTILS_H
