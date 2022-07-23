//
// Created by hongzhe on 22-7-17.
//

#ifndef EUTERPE_FIBER_H
#define EUTERPE_FIBER_H

#include <ucontext.h>
#include <memory>
#include "../thread/euterpe_thread.h"
#include "../thread/mutex.h"
#include <functional>

/// ucontext_t type
/// to load context of a fiber
/// main function:
/// 1. getcontext(ucontext_t*) 2. setcontext(ucontext_t*)
/// 3.void makecontext(ucontext_t*,void(*)(void),int,...) 4. swapcontext()

namespace euterpe {
    class Fiber : public std::enable_shared_from_this<Fiber>{
    public:
        typedef std::shared_ptr<Fiber> ptr;

        enum State {
            /// 初始化状态
            INIT,
            /// 暂停状态
            HOLD,
            /// 执行中状态
            EXEC,
            /// 结束状态
            TERM,
            /// 可执行状态
            READY,
            /// 异常状态
            EXCEPT
        };

        /// 协程状态
        State m_state = INIT;
    private:
        Fiber();
    public:
        /// use_caller 是否在MainFiber上调度
        Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
        ~Fiber();

        /// 重置协程函数和状态
        void reset(std::function<void()> cb);

        /// 抢占线程和让出线程
        void swapIn();
        void swapOut();
        uint64_t getId(){return m_id;};
        State getState() const { return m_state;}

    public:
        /// 返回当前协程
        static Fiber::ptr GetThis();

        /// 设置当前线程的运行协程
        static void SetThis(Fiber* f);

        /// 协程切换到后台并且切换状态
        static void YieldToReady();
        static void YieldToHold();

        /// 获取总携程数
        static uint64_t TotalFibers();

        /// 协程执行函数 执行完成返回到线程主协程
        static void MainFunc();
        void call();
        static void CallerMainFunc();
        static uint64_t GetFiberId();
        void back();

    private:
        /// 协程id
        uint64_t m_id = 0;
        /// 协程运行栈大小
        uint32_t m_stacksize = 0;
        /// 协程上下文
        ucontext_t m_ctx;
        /// 协程运行栈指针
        void* m_stack = nullptr;
        /// 协程运行函数
        std::function<void()> m_cb;

    };
}

#endif //EUTERPE_FIBER_H
