//
// Created by hongzhe on 22-8-6.
//

#include "hook.h"
#include <dlfcn.h>
#include "../Log/log.h"
#include "../coroutines/fiber.h"
#include "../IO/IoManager.h"
#include "../fd_manager/fd_manager.h"

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
    XX(usleep) \
    XX(nanosleep)    \
    XX(socket)       \
    XX(connect) \
    XX(accept)       \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg)      \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg)      \
    XX(close)        \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

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

    struct timer_info {
        int cancelled = 0;
    };

    /**
     * @brief 可变模板参数 c++11新特性
     * @tparam OriginFun
     * @tparam Args
     * @param fd
     * @param fun
     * @param hook_fun_name
     * @param event
     * @param timeout_so
     * @param args
     * @return
     */
    template<typename OriginFun, typename... Args>
    static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
                         uint32_t event, int timeout_so, Args&&... args){
        if(!euterpe::t_hook_enable) {
            return fun(fd, std::forward<Args>(args)...);
        }

        euterpe::FdCtx::ptr ctx = euterpe::FdMgr::GetInstance()->get(fd);
        if(!ctx) {
            return fun(fd, std::forward<Args>(args)...);
        }

        if(ctx->isClose()) {
            errno = EBADF;
            return -1;
        }

        if(!ctx->isSocket() || ctx->getUserNonblock()) {
            return fun(fd, std::forward<Args>(args)...);
        }

        uint64_t to = ctx->getTimeout(timeout_so);
        std::shared_ptr<timer_info> tinfo(new timer_info);

        retry:
        ssize_t n = fun(fd, std::forward<Args>(args)...);
        while(n == -1 && errno == EINTR) {
            n = fun(fd, std::forward<Args>(args)...);
        }
        if(n == -1 && errno == EAGAIN) {
            euterpe::IOManager* iom = euterpe::IOManager::GetThis();
            euterpe::Timer::ptr timer;
            std::weak_ptr<timer_info> winfo(tinfo);

            if(to != (uint64_t)-1) {
                timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                    auto t = winfo.lock();
                    if(!t || t->cancelled) {
                        return;
                    }
                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, (euterpe::IOManager::Event)(event));
                }, winfo);
            }

            int rt = iom->addEvent(fd, (euterpe::IOManager::Event)(event));
            if(EUTERPE_UNLIKELY(rt)) {
                EUTERPE_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                                          << fd << ", " << event << ")";
                if(timer) {
                    timer->cancel();
                }
                return -1;
            } else {
                euterpe::Fiber::YieldToHold();
                if(timer) {
                    timer->cancel();
                }
                if(tinfo->cancelled) {
                    errno = tinfo->cancelled;
                    return -1;
                }
                goto retry;
            }
        }

        return n;
    }

    extern "C" {

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX) ;
#undef XX

    unsigned int sleep(unsigned int seconds) {
        /**
         * @brief 如果不启用 则调用的是原本的函数
         */
        if (!euterpe::t_hook_enable) {
            return sleep_f(seconds);
        }

        /**
         * @brief 直接把当前协程转换为一个定时任务加入IOManager中
         */
        euterpe::Fiber::ptr fiber = euterpe::Fiber::GetThis();
        euterpe::IOManager *iom = euterpe::IOManager::GetThis();

        /**
         * @brief bind的用法 bind(类成员函数，对象，参数列表)
         * @brief 这里在添加了一个定时任务 任务内容是将当协程加入到任务列表中 达到休眠的目的
         * @brief 虽然代码很短 但这里其实是一个很核心的设计
         * @brief 在协程sleep的时候 线程并不会阻塞 而是去分配给了别的协程 大大提高了效率
         * @brief 同理 对于一系列别的socket或者IO的Api 我们同样也进行类似处理
         * @brief 让其基于协程 变为一种异步非阻塞模式 并且调用者无感知 大大提升了效率
         */
        iom->addTimer(seconds * 1000, std::bind((void (euterpe::Scheduler::*)
                (euterpe::Fiber::ptr, int thread)) &euterpe::IOManager::schedule, iom, fiber, -1));

        /// 切换到后台
        euterpe::Fiber::YieldToHold();
        return 0;
    }

    int usleep(useconds_t usec) {
        if (!euterpe::t_hook_enable) {
            return usleep_f(usec);
        }
        euterpe::Fiber::ptr fiber = euterpe::Fiber::GetThis();
        euterpe::IOManager *iom = euterpe::IOManager::GetThis();
        iom->addTimer(usec / 1000, std::bind((void (euterpe::Scheduler::*)
                (euterpe::Fiber::ptr, int thread)) &euterpe::IOManager::schedule, iom, fiber, -1));
        euterpe::Fiber::YieldToHold();
        return 0;
    }

    int nanosleep(const struct timespec *req, struct timespec *rem) {
        if (!euterpe::t_hook_enable) {
            return nanosleep_f(req, rem);
        }

        int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
        euterpe::Fiber::ptr fiber = euterpe::Fiber::GetThis();
        euterpe::IOManager *iom = euterpe::IOManager::GetThis();
        iom->addTimer(timeout_ms, std::bind((void (euterpe::Scheduler::*)
                (euterpe::Fiber::ptr, int thread)) &euterpe::IOManager::schedule, iom, fiber, -1));
        euterpe::Fiber::YieldToHold();
        return 0;
    }
    };
}