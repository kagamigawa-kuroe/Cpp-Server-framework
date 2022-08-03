//
// Created by hongzhe on 22-7-27.
//
#include "IoManager.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <string>


static euterpe::Logger::ptr g_logger = EUTERPE_LOG_NAME("system");

enum EpollCtlOp {
};

static std::ostream& operator<< (std::ostream& os, const EpollCtlOp& op) {
    switch((int)op) {
#define XX(ctl) \
        case ctl: \
            return os << #ctl;
        XX(EPOLL_CTL_ADD);
        XX(EPOLL_CTL_MOD);
        XX(EPOLL_CTL_DEL);
        default:
            return os << (int)op;
    }
#undef XX
}

static std::ostream& operator<< (std::ostream& os, EPOLL_EVENTS events) {
    if(!events) {
        return os << "0";
    }
    bool first = true;
#define XX(E) \
    if(events & E) { \
        if(!first) { \
            os << "|"; \
        } \
        os << #E; \
        first = false; \
    }
    XX(EPOLLIN);
    XX(EPOLLPRI);
    XX(EPOLLOUT);
    XX(EPOLLRDNORM);
    XX(EPOLLRDBAND);
    XX(EPOLLWRNORM);
    XX(EPOLLWRBAND);
    XX(EPOLLMSG);
    XX(EPOLLERR);
    XX(EPOLLHUP);
    XX(EPOLLRDHUP);
    XX(EPOLLONESHOT);
    XX(EPOLLET);
#undef XX
    return os;
}

euterpe::IOManager::FdContext::EventContext &
euterpe::IOManager::FdContext::getContext(euterpe::IOManager::Event event) {
    switch(event) {
        case IOManager::READ:
            return read;
        case IOManager::WRITE:
            return write;
        default:
            EUTERPE_ASSERT2(false, "getContext");
    }
}

void euterpe::IOManager::FdContext::resetContext(euterpe::IOManager::FdContext::EventContext &ctx) {
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}

/// 将任务schedule到任务列表
void euterpe::IOManager::FdContext::triggerEvent(euterpe::IOManager::Event event) {
    EUTERPE_ASSERT(events & event);
    /// 删掉执行的event
    events = (Event)(events & ~event);
    EventContext& ctx = getContext(event);
    if(ctx.cb) {
        ctx.scheduler->schedule(&ctx.cb);
    } else {
        ctx.scheduler->schedule(&ctx.fiber);
    }
    ctx.scheduler = nullptr;
    return;
}

/// ------------------------------------------------------------------------------------------

/// 构造函数 初始化epoll和管道
euterpe::IOManager::IOManager(size_t threads, bool use_caller, const std::string &name)
:Scheduler(threads, use_caller,name) {
    m_epfd = epoll_create(5000);
    EUTERPE_ASSERT(m_epfd > 0);

    int rt = pipe(m_tickleFds);
    EUTERPE_ASSERT(!rt);

    epoll_event event;
    /// 内存初始化
    /// 将某一块内存中的内容全部设置为指定的值
    memset(&event, 0, sizeof(epoll_event));

    /// ET模式 只会通知一次 不会再次触发
    /// EPOLLIN 读事件
    event.events = EPOLLIN | EPOLLET;

    /// pipe()创建管道后读端对应的文件描述符为fd[0]，写端对应的文件描述符为fd[1]
    event.data.fd = m_tickleFds[0];

    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    EUTERPE_ASSERT(!rt);

    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    EUTERPE_ASSERT(!rt);

    contextResize(32);

    start();
}

/// 释放所有的内容 包括epoll 管道 所有注册时间
euterpe::IOManager::~IOManager() {
    stop();
    /// epoll描述符关闭时 会触发17 1+2+4+10 也就是读+写+紧急事件+挂起
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
}

/// resize m_fdContexts
void euterpe::IOManager::contextResize(size_t size) {
    m_fdContexts.resize(size);

    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}

/// 添加并注册一个fd 以及对应的事件
int euterpe::IOManager::addEvent(int fd, euterpe::IOManager::Event event, std::function<void()> cb) {
    FdContext* fd_ctx = nullptr;
    RWMutexType::ReadLock lock(m_mutex);
    /// 如果原本这个fd就已经被注册了
    /// 注意在resize的时候 会直接根据数组的下标注册fd
    /// 所以只要m_fdContexts的大小大于现在要注册的fd
    /// 那么这个fd一定已经存在了
    /// 如果不存在 说明数组长度不够 我们扩容即可
    if((int)m_fdContexts.size() > fd) {
        fd_ctx = m_fdContexts[fd];
        lock.unlock();
    } else {
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_fdContexts[fd];
    }

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(EUTERPE_UNLIKELY(fd_ctx->events & event)) {
        EUTERPE_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                                  << " event=" << (EPOLL_EVENTS)event
                                  << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events;
        EUTERPE_ASSERT(!(fd_ctx->events & event));
    }

    /// 如果这个fd在容器中已经存在
    /// 为我们的op就是修改 不然就是添加
    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->events | event;
    epevent.data.ptr = fd_ctx;

    /// 注册进epoll
    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        EUTERPE_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                  << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
                                  << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
                                  << (EPOLL_EVENTS)fd_ctx->events;
        return -1;
    }

    /// 记数增加
    ++m_pendingEventCount;
    /// 修改容器中事件的状态 读/写
    fd_ctx->events = (Event)(fd_ctx->events | event);

    /// 根据读或者写 拿到FdContext不同的内容
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    EUTERPE_ASSERT(!event_ctx.scheduler
                 && !event_ctx.fiber
                 && !event_ctx.cb);

    /// 设置scheduler
    event_ctx.scheduler = Scheduler::GetThis();
    /// 设置函数或者fiber
    if(cb) {
        event_ctx.cb.swap(cb);
    } else {
        event_ctx.fiber = Fiber::GetThis();
        EUTERPE_ASSERT2(event_ctx.fiber->getState() == Fiber::EXEC
                ,"state=" << event_ctx.fiber->getState());
    }
    return 0;
}

/// 删除一个事件 不会触发事件
bool euterpe::IOManager::delEvent(int fd, euterpe::IOManager::Event event) {
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    /// 拿到fd对应的FdContext
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(EUTERPE_UNLIKELY(!(fd_ctx->events & event))) {
        return false;
    }

    /// 新事件 将传入想要删除的旧事件用位运算删除
    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    /// 重新在epoll中的注册 没有则是删除
    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        EUTERPE_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                  << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
                                  << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    --m_pendingEventCount;
    fd_ctx->events = new_events;
    /// 将原先的event对应的事件read/write重置
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}

/// 删除一个事件 会触发事件
bool euterpe::IOManager::cancelEvent(int fd, euterpe::IOManager::Event event) {
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(EUTERPE_UNLIKELY(!(fd_ctx->events & event))) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        EUTERPE_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                  << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
                                  << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    /// 比起del cancel会调用triggerEvent函数 触发对应内容
    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}

/// 取消所有
bool euterpe::IOManager::cancelAll(int fd) {
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!fd_ctx->events) {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        EUTERPE_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                  << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
                                  << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    if(fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if(fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }

    EUTERPE_ASSERT(fd_ctx->events == 0);
    return true;
}

/// 类似scheduler的GetThis()
euterpe::IOManager *euterpe::IOManager::GetThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void euterpe::IOManager::tickle() {
    if(!hasIdleThreads()) {
        return;
    }
    /// 向管道写一个数据 告诉manage 用于唤醒epollwait
    int rt = write(m_tickleFds[1], "T", 1);
    EUTERPE_ASSERT(rt == 1);
}

bool euterpe::IOManager::stopping() {
    return Scheduler::stopping()&& m_pendingEventCount==0;
}

void euterpe::IOManager::idle() {
    EUTERPE_LOG_DEBUG(g_logger) << "idle";
    const uint64_t MAX_EVNETS = 256;
    epoll_event* events = new epoll_event[MAX_EVNETS]();
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
        delete[] ptr;
    });

    while(true) {
        // uint64_t next_timeout = 0;
        if(stopping()) {
            EUTERPE_LOG_INFO(g_logger) << "name=" << getName()
                                     << " idle stopping exit";
            break;
        }

        int rt = 0;
        do {
            static const int MAX_TIMEOUT = 3000;
//            if(next_timeout != ~0ull) {
//                next_timeout = (int)next_timeout > MAX_TIMEOUT
//                               ? MAX_TIMEOUT : next_timeout;
//            } else {
//                next_timeout = MAX_TIMEOUT;
//            }
            rt = epoll_wait(m_epfd, events, MAX_EVNETS, MAX_TIMEOUT);
            if(rt < 0 && errno == EINTR) {
            } else {
                break;
            }
        } while(true);

//        std::vector<std::function<void()> > cbs;
//        listExpiredCb(cbs);
//        if(!cbs.empty()) {
//            //SYLAR_LOG_DEBUG(g_logger) << "on timer cbs.size=" << cbs.size();
//            schedule(cbs.begin(), cbs.end());
//            cbs.clear();
//        }

        //if(SYLAR_UNLIKELY(rt == MAX_EVNETS)) {
        //    SYLAR_LOG_INFO(g_logger) << "epoll wait events=" << rt;
        //}

        for(int i = 0; i < rt; ++i) {
            epoll_event& event = events[i];

            /// 如果只是提醒唤醒 那么就跳过
            if(event.data.fd == m_tickleFds[0]) {
                uint8_t dummy[256];
                while(read(m_tickleFds[0], dummy, sizeof(dummy)) > 0);
                continue;
            }

            FdContext* fd_ctx = (FdContext*)event.data.ptr;
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            if(event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
            }
            int real_events = NONE;
            // std::cout << event.events << std::endl;
            if(event.events & EPOLLIN) {
                real_events |= READ;
            }
            if(event.events & EPOLLOUT) {
                real_events |= WRITE;
            }

            if((fd_ctx->events & real_events) == NONE) {
                continue;
            }

            int left_events = (fd_ctx->events & ~real_events);
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;

            /// 处理完一种触发后 把剩下的重新注册回去
            int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if(rt2) {
                EUTERPE_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                          << (EpollCtlOp)op << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                                          << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
            }

            if(real_events & READ) {
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if(real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        }

        /// 归还调度权给run函数
        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        raw_ptr->swapOut();
    }
}

//bool euterpe::IOManager::stopping(uint64_t &timeout) {
//    return false;
//}
