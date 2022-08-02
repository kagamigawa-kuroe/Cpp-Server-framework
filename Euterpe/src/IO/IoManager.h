//
// Created by hongzhe on 22-7-27.
//

#ifndef EUTERPE_IOMANAGER_H
#define EUTERPE_IOMANAGER_H

#include "../scheduler/scheduler.h"
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

namespace euterpe{
    class IOManager:public Scheduler{
    public:
        typedef std::shared_ptr<IOManager> ptr;
        typedef RWMutex RWMutexType;

        enum Event {
            /// 无事件
            NONE    = 0x000,
            /// 读事件(EPOLLIN)
            READ    = 0x001,
            /// 写事件(EPOLLOUT)
            WRITE   = 0x004,
        };
    private:
        /// 一个FdContext代表一个IO描述符 拥有两种内容 一种为读 一种写
        /// 分别对应了两种 EventContext 对应读写的触发 储存了不同的函数
        /// events记录了当前的触发的事件
        /// 会根据event的内容 调用不同的EventContext的内容
        struct FdContext {
            typedef Mutex MutexType;
            struct EventContext {
                /// 事件待执行的调度器
                Scheduler* scheduler = nullptr;
                /// 事件协程
                Fiber::ptr fiber;
                /// 事件的回调函数
                std::function<void()> cb;
            };

            /// 根据当前事件中的状态 返回当前事件中的内容read/write
            EventContext& getContext(Event event);

            /// 重置当前事件中的内容
            void resetContext(EventContext& ctx);

            /// 触发一个事件
            /// 会把event对应的事件加入到fiber队列中
            /// 比如当epoll被读事件唤醒后 我们拿到读事件fd对应的FdContext
            /// 然后把FdContext对应的read内容加入到fiber队列中
            void triggerEvent(Event event);

            /// 读事件内容
            EventContext read;
            /// 写事件内容
            EventContext write;

            /// 事件关联的句柄 事件的描述符
            int fd = 0;
            /// 当前的事件
            Event events = NONE;
            /// 事件的Mutex
            MutexType mutex;
        };

    public:
        /**
         * @brief 构造函数
         * @param[in] threads 线程数量
         * @param[in] use_caller 是否将调用线程包含进去
         * @param[in] name 调度器的名称
         */
        IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");


        ~IOManager();
        /// 注册事件的增删改查
        int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
        bool delEvent(int fd, Event event);
        bool cancelEvent(int fd, Event event);
        bool cancelAll(int fd);
        static IOManager* GetThis();

        /// 重写继承的scheduler类中的一系列方法
        void tickle() override;
        bool stopping() override;
        void idle() override;


        /// 重置socket句柄上下文的容器大小
        void contextResize(size_t size);

        /// 判断是否可以停止
        bool stopping(uint64_t& timeout);

    private:
        /// epoll 文件句柄
        int m_epfd = 0;
        /// pipe 文件句柄 也会被注册进epoll中
        /// idle epoll_wait 陷入沉睡的时候 可以用这个管道来唤醒
        int m_tickleFds[2];
        /// 当前等待执行的事件数量  EventContext
        std::atomic<size_t> m_pendingEventCount = {0};
        /// IOManager的Mutex
        RWMutexType m_mutex;
        /// socket所有的注册事件
        std::vector<FdContext*> m_fdContexts;

    };
}

#endif //EUTERPE_IOMANAGER_H
