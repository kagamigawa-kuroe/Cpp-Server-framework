//
// Created by hongzhe on 22-7-27.
//
#include "IoManager.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

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

}

void euterpe::IOManager::FdContext::resetContext(euterpe::IOManager::FdContext::EventContext &ctx) {

}

void euterpe::IOManager::FdContext::triggerEvent(euterpe::IOManager::Event event) {

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

int euterpe::IOManager::addEvent(int fd, euterpe::IOManager::Event event, std::function<void()> cb) {
    FdContext* fd_ctx = nullptr;
    RWMutexType::ReadLock lock(m_mutex);
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

    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->events | event;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        EUTERPE_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                  << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
                                  << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
                                  << (EPOLL_EVENTS)fd_ctx->events;
        return -1;
    }

    ++m_pendingEventCount;
    fd_ctx->events = (Event)(fd_ctx->events | event);
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    EUTERPE_ASSERT(!event_ctx.scheduler
                 && !event_ctx.fiber
                 && !event_ctx.cb);

    event_ctx.scheduler = Scheduler::GetThis();
    if(cb) {
        event_ctx.cb.swap(cb);
    } else {
        event_ctx.fiber = Fiber::GetThis();
//        SYLAR_ASSERT2(event_ctx.fiber->getState() == Fiber::EXEC
//                ,"state=" << event_ctx.fiber->getState());
    }
    return 0;
}

bool euterpe::IOManager::delEvent(int fd, euterpe::IOManager::Event event) {
    return false;
}

bool euterpe::IOManager::cancelEvent(int fd, euterpe::IOManager::Event event) {
    return false;
}

bool euterpe::IOManager::cancelAll(int fd) {
    return false;
}

euterpe::IOManager *euterpe::IOManager::GetThis() {
    return nullptr;
}

void euterpe::IOManager::tickle() {
    Scheduler::tickle();
}

bool euterpe::IOManager::stopping() {
    return Scheduler::stopping();
}

void euterpe::IOManager::idle() {
    Scheduler::idle();
}

bool euterpe::IOManager::stopping(uint64_t &timeout) {
    return false;
}
