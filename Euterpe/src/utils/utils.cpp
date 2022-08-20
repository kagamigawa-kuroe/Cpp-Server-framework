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
#include <execinfo.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <csignal>
#include "../coroutines/fiber.h"
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
        return euterpe::Fiber::GetFiberId();
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

    uint64_t GetCurrentMS() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000ul  + tv.tv_usec / 1000;
    }

    uint64_t GetCurrentUS() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000 * 1000ul  + tv.tv_usec;
    }

    void FSUtil::ListAllFile(std::vector<std::string>& files
            ,const std::string& path
            ,const std::string& subfix) {
        if(access(path.c_str(), 0) != 0) {
            return;
        }
        DIR* dir = opendir(path.c_str());
        if(dir == nullptr) {
            return;
        }
        struct dirent* dp = nullptr;
        while((dp = readdir(dir)) != nullptr) {
            if(dp->d_type == DT_DIR) {
                if(!strcmp(dp->d_name, ".")
                   || !strcmp(dp->d_name, "..")) {
                    continue;
                }
                ListAllFile(files, path + "/" + dp->d_name, subfix);
            } else if(dp->d_type == DT_REG) {
                std::string filename(dp->d_name);
                if(subfix.empty()) {
                    files.push_back(path + "/" + filename);
                } else {
                    if(filename.size() < subfix.size()) {
                        continue;
                    }
                    if(filename.substr(filename.length() - subfix.size()) == subfix) {
                        files.push_back(path + "/" + filename);
                    }
                }
            }
        }
        closedir(dir);
    }

    static int __lstat(const char* file, struct stat* st = nullptr) {
        struct stat lst;
        int ret = lstat(file, &lst);
        if(st) {
            *st = lst;
        }
        return ret;
    }

    static int __mkdir(const char* dirname) {
        if(access(dirname, F_OK) == 0) {
            return 0;
        }
        return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    bool FSUtil::Mkdir(const std::string& dirname) {
        if(__lstat(dirname.c_str()) == 0) {
            return true;
        }
        char* path = strdup(dirname.c_str());
        char* ptr = strchr(path + 1, '/');
        do {
            for(; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
                *ptr = '\0';
                if(__mkdir(path) != 0) {
                    break;
                }
            }
            if(ptr != nullptr) {
                break;
            } else if(__mkdir(path) != 0) {
                break;
            }
            free(path);
            return true;
        } while(0);
        free(path);
        return false;
    }

    bool FSUtil::IsRunningPidfile(const std::string& pidfile) {
        if(__lstat(pidfile.c_str()) != 0) {
            return false;
        }
        std::ifstream ifs(pidfile);
        std::string line;
        if(!ifs || !std::getline(ifs, line)) {
            return false;
        }
        if(line.empty()) {
            return false;
        }
        pid_t pid = atoi(line.c_str());
        if(pid <= 1) {
            return false;
        }
        if(kill(pid, 0) != 0) {
            return false;
        }
        return true;
    }

    bool FSUtil::Unlink(const std::string& filename, bool exist) {
        if(!exist && __lstat(filename.c_str())) {
            return true;
        }
        return ::unlink(filename.c_str()) == 0;
    }

    bool FSUtil::Rm(const std::string& path) {
        struct stat st;
        if(lstat(path.c_str(), &st)) {
            return true;
        }
        if(!(st.st_mode & S_IFDIR)) {
            return Unlink(path);
        }

        DIR* dir = opendir(path.c_str());
        if(!dir) {
            return false;
        }

        bool ret = true;
        struct dirent* dp = nullptr;
        while((dp = readdir(dir))) {
            if(!strcmp(dp->d_name, ".")
               || !strcmp(dp->d_name, "..")) {
                continue;
            }
            std::string dirname = path + "/" + dp->d_name;
            ret = Rm(dirname);
        }
        closedir(dir);
        if(::rmdir(path.c_str())) {
            ret = false;
        }
        return ret;
    }

    bool FSUtil::Mv(const std::string& from, const std::string& to) {
        if(!Rm(to)) {
            return false;
        }
        return rename(from.c_str(), to.c_str()) == 0;
    }

    bool FSUtil::Realpath(const std::string& path, std::string& rpath) {
        if(__lstat(path.c_str())) {
            return false;
        }
        char* ptr = ::realpath(path.c_str(), nullptr);
        if(nullptr == ptr) {
            return false;
        }
        std::string(ptr).swap(rpath);
        free(ptr);
        return true;
    }

    bool FSUtil::Symlink(const std::string& from, const std::string& to) {
        if(!Rm(to)) {
            return false;
        }
        return ::symlink(from.c_str(), to.c_str()) == 0;
    }

    std::string FSUtil::Dirname(const std::string& filename) {
        if(filename.empty()) {
            return ".";
        }
        auto pos = filename.rfind('/');
        if(pos == 0) {
            return "/";
        } else if(pos == std::string::npos) {
            return ".";
        } else {
            return filename.substr(0, pos);
        }
    }

    std::string FSUtil::Basename(const std::string& filename) {
        if(filename.empty()) {
            return filename;
        }
        auto pos = filename.rfind('/');
        if(pos == std::string::npos) {
            return filename;
        } else {
            return filename.substr(pos + 1);
        }
    }

    bool FSUtil::OpenForRead(std::ifstream& ifs, const std::string& filename
            ,std::ios_base::openmode mode) {
        ifs.open(filename.c_str(), mode);
        return ifs.is_open();
    }

    bool FSUtil::OpenForWrite(std::ofstream& ofs, const std::string& filename
            ,std::ios_base::openmode mode) {
        ofs.open(filename.c_str(), mode);
        if(!ofs.is_open()) {
            std::string dir = Dirname(filename);
            Mkdir(dir);
            ofs.open(filename.c_str(), mode);
        }
        return ofs.is_open();
    }
}