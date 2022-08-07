//
// Created by hongzhe on 22-8-6.
//

#include "hook.h"
#include <dlfcn.h>
#include "../Log/log.h"
#include "../coroutines/fiber.h"
#include "../IO/IoManager.h"

euterpe::Logger::ptr g_logger = EUTERPE_LOG_NAME("system");

namespace euterpe {

    /**
     * @brief 线程全局变量 表示是否启用hook
     */
    static thread_local bool t_hook_enable = false;

//
//    static euterpe::ConfigVar<int>::ptr g_tcp_connect_timeout =
//            euterpe::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

    /**
     * @brief 所有hook的函数
     */
#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep)

    void hook_init() {
        static bool is_inited = false;
        if (is_inited) {
            return;
        }
        /**
         * @brief name为原函数的名称 name_fun为原函数对应的函数指针的类型
         * @brief name_f 为原函数的拷贝
         * @brief e.g. sleep_f = (sleep_fun)dlsym(RTLD_NEXT,sleep);
         */
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(XX);
#undef XX
    }

    static uint64_t s_connect_timeout = -1;
    struct _HookIniter {
        _HookIniter() {
            hook_init();
//            s_connect_timeout = g_tcp_connect_timeout->getValue();
//
//            g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
//                EUTERPE_LOG_INFO(g_logger) << "tcp connect timeout changed from "
//                                         << old_value << " to " << new_value;
//                s_connect_timeout = new_value;
//            });
        }
    };

    static _HookIniter s_hook_initer;

    bool is_hook_enable() {
        return t_hook_enable;
    }

    void set_hook_enable(bool flag) {
        t_hook_enable = flag;
    }
}

extern "C" {

#define XX(name) name ## _fun name ## _f = nullptr;
HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    /**
     * @brief 如果不启用 则调用的是原本的函数
     */
    if(!euterpe::t_hook_enable) {
        return sleep_f(seconds);
    }

    /**
     * @brief 直接把当前协程转换为一个定时任务加入IOManager中
     */
    euterpe::Fiber::ptr fiber = euterpe::Fiber::GetThis();
    euterpe::IOManager* iom = euterpe::IOManager::GetThis();
    iom->addTimer(seconds * 1000, std::bind((void(euterpe::Scheduler::*)
                                                    (euterpe::Fiber::ptr, int thread))&euterpe::IOManager::schedule
            ,iom, fiber, -1));

    /// 切换到后台
    euterpe::Fiber::YieldToHold();
    return 0;
}

/**
 * @brief 逻辑同上
 * @param usec
 * @return
 */
int usleep(useconds_t usec) {
    if(!euterpe::t_hook_enable) {
        return usleep_f(usec);
    }
    euterpe::Fiber::ptr fiber = euterpe::Fiber::GetThis();
    euterpe::IOManager* iom = euterpe::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(euterpe::Scheduler::*)
                                                 (euterpe::Fiber::ptr, int thread))&euterpe::IOManager::schedule
            ,iom, fiber, -1));
    euterpe::Fiber::YieldToHold();
    return 0;
}
}

