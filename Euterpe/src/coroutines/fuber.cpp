//
// Created by hongzhe on 22-7-18.
//

#include "fiber.h"
#include "../Log/log.h"
#include "../config/config.h"
#include "../utils/macro.h"
#include "../scheduler/scheduler.h"
#include <atomic>
namespace euterpe{

    static Logger::ptr g_logger = EUTERPE_LOG_NAME("system");
    static std::atomic<uint64_t> s_fiber_id {0};
    static std::atomic<uint64_t> s_fiber_count {0};

    static thread_local Fiber* t_fiber = nullptr;

    /// main fiber
    static thread_local Fiber::ptr t_threadFiber = nullptr;

    static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
            Config::Lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

    /// 内存分配器
    class MallocStackAllocator {
    public:
        static void* Alloc(size_t size) {
            return malloc(size);
        }

        static void Dealloc(void* vp, size_t size) {
            return free(vp);
        }
    };
    using StackAllocator = MallocStackAllocator;

    Fiber::Fiber(){
        m_state = EXEC;
        SetThis(this);
        if(getcontext(&m_ctx)){
            EUTERPE_ASSERT2(false, "getcontext");
        }
        ++s_fiber_count;
    };

    /// 设置当前协程
    void Fiber::SetThis(Fiber* f) {
        t_fiber = f;
    }

    Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
            :m_id(++s_fiber_id)
            ,m_cb(cb) {
        ++s_fiber_count;
        m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

        m_stack = StackAllocator::Alloc(m_stacksize);
        if(getcontext(&m_ctx)) {
            EUTERPE_ASSERT2(false, "getcontext");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        if(!use_caller) {
            makecontext(&m_ctx, &Fiber::MainFunc, 0);
        } else {
            makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
        }

        EUTERPE_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
    }

    Fiber::~Fiber(){
        --s_fiber_count;
        if(m_stack) {
            EUTERPE_ASSERT(m_state == TERM
                         || m_state == EXCEPT
                         || m_state == INIT);

            StackAllocator::Dealloc(m_stack, m_stacksize);
        } else {
            EUTERPE_ASSERT(!m_cb);
            EUTERPE_ASSERT(m_state == EXEC);

            Fiber* cur = t_fiber;
            if(cur == this) {
                SetThis(nullptr);
            }
        }
        EUTERPE_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
                                  << " total=" << s_fiber_count;
    };

    /// 重置协程函数和状态
    void Fiber::reset(std::function<void()> cb){
        EUTERPE_ASSERT(m_stack);
        EUTERPE_ASSERT(m_state == TERM
                     || m_state == EXCEPT
                     || m_state == INIT);
        m_cb = cb;

        /// reset object context
        if(getcontext(&m_ctx)) {
            EUTERPE_ASSERT2(false, "getcontext");
        }

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        m_state = INIT;
    };

    /// 返回当前协程
    Fiber::ptr Fiber::GetThis() {
        if(t_fiber) {
            return t_fiber->shared_from_this();
        }
        Fiber::ptr main_fiber(new Fiber);
        EUTERPE_ASSERT(t_fiber == main_fiber.get());
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }


    //协程切换到后台，并且设置为Ready状态
    void Fiber::YieldToReady() {
        Fiber::ptr cur = GetThis();
        // EUTERPE_ASSERT(cur->m_state == EXEC);
        cur->m_state = READY;
        cur->swapOut();
    }

//协程切换到后台，并且设置为Hold状态
    void Fiber::YieldToHold() {
        Fiber::ptr cur = GetThis();

        cur->m_state = HOLD;
        cur->swapOut();
    }

//总协程数
    uint64_t Fiber::TotalFibers() {
        return s_fiber_count;
    }

    /// 协程执行函数 执行完成返回到线程主协程
    void Fiber::MainFunc(){
        Fiber::ptr cur = GetThis();
        EUTERPE_ASSERT(cur);
        try {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        } catch (std::exception& ex) {
            cur->m_state = EXCEPT;
            EUTERPE_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                                      << " fiber_id=" << cur->getId()
                                      << std::endl
                                      << euterpe::BacktraceToString();
        } catch (...) {
            cur->m_state = EXCEPT;
            EUTERPE_LOG_ERROR(g_logger) << "Fiber Except"
                                      << " fiber_id=" << cur->getId()
                                      << std::endl
                                      << euterpe::BacktraceToString();
        }

        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->swapOut();

        EUTERPE_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
    }

    /// 执行完成返回到线程调度协程
    void Fiber::CallerMainFunc() {
        Fiber::ptr cur = GetThis();
        EUTERPE_ASSERT(cur);
        try {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        } catch (std::exception& ex) {
            cur->m_state = EXCEPT;
            EUTERPE_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                                      << " fiber_id=" << cur->getId()
                                      << std::endl
                                      << euterpe::BacktraceToString();
        } catch (...) {
            cur->m_state = EXCEPT;
            EUTERPE_LOG_ERROR(g_logger) << "Fiber Except"
                                      << " fiber_id=" << cur->getId()
                                      << std::endl
                                      << euterpe::BacktraceToString();
        }

        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->back();
        EUTERPE_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));

    }

    void Fiber::back() {
        SetThis(t_threadFiber.get());
        if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
            EUTERPE_ASSERT2(false, "swapcontext");
        }
    }

    /// 切换到当前协程执行
    void Fiber::swapIn() {
        SetThis(this);
        EUTERPE_ASSERT(m_state != EXEC);
        m_state = EXEC;
        if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
            EUTERPE_ASSERT2(false, "swapcontext");
        }
    }

    /// 切换到后台执行
    void Fiber::swapOut() {
        SetThis(Scheduler::GetMainFiber());
        if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
            EUTERPE_ASSERT2(false, "swapcontext");
        }
    }

    uint64_t Fiber::GetFiberId() {
        if(t_fiber){
            return t_fiber->getId();
        }
        return 0;
    }

    void Fiber::call() {
        SetThis(this);
        m_state = EXEC;
        if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
            SYLAR_ASSERT2(false, "swapcontext");
        }
    }
}