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


    static euterpe::ConfigVar<int>::ptr g_tcp_connect_timeout =
            euterpe::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

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
            s_connect_timeout = g_tcp_connect_timeout->getValue();

            g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
                EUTERPE_LOG_INFO(g_logger) << "tcp connect timeout changed from "
                                         << old_value << " to " << new_value;
                s_connect_timeout = new_value;
            });
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
        /// 如果线程没有启动hook 就执行原函数
        if(!euterpe::t_hook_enable) {
            return fun(fd, std::forward<Args>(args)...);
        }

        /// 返回一个单例FdCtx对象的智能指针
        euterpe::FdCtx::ptr ctx = euterpe::FdMgr::GetInstance()->get(fd);

        /// 没拿到 就直接执行原函数
        if(!ctx) {
            return fun(fd, std::forward<Args>(args)...);
        }

        /// 如果FdCtx已经被关闭了 返回错误
        if(ctx->isClose()) {
            errno = EBADF;
            return -1;
        }

        /// 如果不是socket 或者 是用户主动设置的非阻塞
        /// 直接执行
        if(!ctx->isSocket() || ctx->getUserNonblock()) {
            return fun(fd, std::forward<Args>(args)...);
        }

        uint64_t to = ctx->getTimeout(timeout_so);
        std::shared_ptr<timer_info> tinfo(new timer_info);

        /// 执行函数 并获取返回值
        retry:

        /// 如果这个函数返回值为-1 且error为EINITR(Interrupted system call) 说明函数运行出错 需要重新运行
        ssize_t n = fun(fd, std::forward<Args>(args)...);
        while(n == -1 && errno == EINTR) {
            n = fun(fd, std::forward<Args>(args)...);
        }

        /// 如果这个函数返回值为-1 且error为EAGAIN(Try again) 说明还没有准备好
        /// 在非阻塞情况下 我们需要切换协程 将其作为一个定时任务加入到定时器中
        /// 一段时间以后再次尝试执行

        /// 例如 accept函数 在非阻塞情况下 会返回-1 当我们拿到-1时 就把他加入到epoll中
        /// 接下来 就可以让出协程 去做别的事情
        /// 当fd文件准备好 也就是可以使用accept连接时 idle协程会唤醒
        if(n == -1 && errno == EAGAIN) {
            euterpe::IOManager* iom = euterpe::IOManager::GetThis();
            euterpe::Timer::ptr timer;
            std::weak_ptr<timer_info> winfo(tinfo);

            if(to != (uint64_t)-1) {
                /// 添加定时任务 一段时间后取消这个fd事件
                timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                    auto t = winfo.lock();
                    if(!t || t->cancelled) {
                        return;
                    }
                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, (euterpe::IOManager::Event)(event));
                }, winfo);
            }

            /// 添加一个事件 也就是把这个fd注册进epoll 这样他好了 我们才会去进行操作
            /// 第三个参数为缺省值 也就是注册当前fiber
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

    int socket(int domain, int type, int protocol) {
        if(!euterpe::t_hook_enable) {
            return socket_f(domain, type, protocol);
        }
        int fd = socket_f(domain, type, protocol);
        if(fd == -1) {
            return fd;
        }
        euterpe::FdMgr::GetInstance()->get(fd, true);
        return fd;
    }

    int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) {
        /// 如果没有启动hook则直接调用原生connect
        if(!euterpe::t_hook_enable) {
            return connect_f(fd, addr, addrlen);
        }

        /// 拿到文件描述符 没有则新建
        euterpe::FdCtx::ptr ctx = euterpe::FdMgr::GetInstance()->get(fd);
        if(!ctx || ctx->isClose()) {
            errno = EBADF;
            return -1;
        }

        if(!ctx->isSocket()) {
            return connect_f(fd, addr, addrlen);
        }

        if(ctx->getUserNonblock()) {
            return connect_f(fd, addr, addrlen);
        }

        int n = connect_f(fd, addr, addrlen);
        if(n == 0) {
            return 0;
        } else if(n != -1 || errno != EINPROGRESS) {
            return n;
        }

        /// 如果connect阻塞 返回-1 则会继续往下
        euterpe::IOManager* iom = euterpe::IOManager::GetThis();
        euterpe::Timer::ptr timer;
        std::shared_ptr<timer_info> tinfo(new timer_info);
        std::weak_ptr<timer_info> winfo(tinfo);

        /// 如果有过期时间 添加一个定时的取消任务
        if(timeout_ms != (uint64_t)-1) {
            timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, euterpe::IOManager::WRITE);
            }, winfo);
        }

        /// 在epoll注册一个写事件 当这个文件描述符可写 也就是与服务器之间的连接建立 就会触发
        /// 然后就会切换回当前协程
        int rt = iom->addEvent(fd, euterpe::IOManager::WRITE);
        if(rt == 0) {
            euterpe::Fiber::YieldToHold();
            if(timer) {
                timer->cancel();
            }
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
        } else {
            if(timer) {
                timer->cancel();
            }
            EUTERPE_LOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
        }

        int error = 0;
        socklen_t len = sizeof(int);
        if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
            return -1;
        }
        if(!error) {
            return 0;
        } else {
            errno = error;
            return -1;
        }
    }

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        return connect_with_timeout(sockfd, addr, addrlen, euterpe::s_connect_timeout);
    }

    int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
        int fd = do_io(s, accept_f, "accept", euterpe::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
        if(fd >= 0) {
            euterpe::FdMgr::GetInstance()->get(fd, true);
        }
        return fd;
    }

    ssize_t read(int fd, void *buf, size_t count) {
        return do_io(fd, read_f, "read", euterpe::IOManager::READ, SO_RCVTIMEO, buf, count);
    }

    ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
        return do_io(fd, readv_f, "readv", euterpe::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
    }

    ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
        return do_io(sockfd, recv_f, "recv", euterpe::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
    }

    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
        return do_io(sockfd, recvfrom_f, "recvfrom", euterpe::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
    }

    ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
        return do_io(sockfd, recvmsg_f, "recvmsg", euterpe::IOManager::READ, SO_RCVTIMEO, msg, flags);
    }

    ssize_t write(int fd, const void *buf, size_t count) {
        return do_io(fd, write_f, "write", euterpe::IOManager::WRITE, SO_SNDTIMEO, buf, count);
    }

    ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
        return do_io(fd, writev_f, "writev", euterpe::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
    }

    ssize_t send(int s, const void *msg, size_t len, int flags) {
        return do_io(s, send_f, "send", euterpe::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
    }

    ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
        return do_io(s, sendto_f, "sendto", euterpe::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
    }

    ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
        return do_io(s, sendmsg_f, "sendmsg", euterpe::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
    }

    int close(int fd) {
        if(!euterpe::t_hook_enable) {
            return close_f(fd);
        }

        euterpe::FdCtx::ptr ctx = euterpe::FdMgr::GetInstance()->get(fd);
        if(ctx) {
            auto iom = euterpe::IOManager::GetThis();
            if(iom) {
                iom->cancelAll(fd);
            }
            euterpe::FdMgr::GetInstance()->del(fd);
        }
        return close_f(fd);
    }

    /**
     * @brief 更改文件描述符的性质
     * @param fd
     * @param cmd
     * @param ...
     * @return
     */
    int fcntl(int fd, int cmd, ... /* arg */ ) {
        va_list va;
        va_start(va, cmd);
        switch(cmd) {
            case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                euterpe::FdCtx::ptr ctx = euterpe::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
                break;
            case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                euterpe::FdCtx::ptr ctx = euterpe::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
                break;
            case F_DUPFD:
            case F_DUPFD_CLOEXEC:
            case F_SETFD:
            case F_SETOWN:
            case F_SETSIG:
            case F_SETLEASE:
            case F_NOTIFY:
#ifdef F_SETPIPE_SZ
            case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
                break;
            case F_GETFD:
            case F_GETOWN:
            case F_GETSIG:
            case F_GETLEASE:
#ifdef F_GETPIPE_SZ
            case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
                break;
            case F_SETLK:
            case F_SETLKW:
            case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
                break;
            case F_GETOWN_EX:
            case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
                break;
            default:
                va_end(va);
                return fcntl_f(fd, cmd);
        }
    }

    /**
     * @brief 在涉及到有关于io描述性质更改的内容时
     * @brief 都需要将所有注册在FdMgr中的IO描述符进行一个更改
     * @param d
     * @param request
     * @param ...
     * @return
     */
    int ioctl(int d, unsigned long int request, ...) {
        va_list va;
        va_start(va, request);
        void* arg = va_arg(va, void*);
        va_end(va);

        if(FIONBIO == request) {
            bool user_nonblock = !!*(int*)arg;
            euterpe::FdCtx::ptr ctx = euterpe::FdMgr::GetInstance()->get(d);
            if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                return ioctl_f(d, request, arg);
            }
            ctx->setUserNonblock(user_nonblock);
        }
        return ioctl_f(d, request, arg);
    }

    int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
        return getsockopt_f(sockfd, level, optname, optval, optlen);
    }

    int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
        if(!euterpe::t_hook_enable) {
            return setsockopt_f(sockfd, level, optname, optval, optlen);
        }
        if(level == SOL_SOCKET) {
            if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
                euterpe::FdCtx::ptr ctx = euterpe::FdMgr::GetInstance()->get(sockfd);
                if(ctx) {
                    const timeval* v = (const timeval*)optval;
                    ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
                }
            }
        }
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }

    };
}