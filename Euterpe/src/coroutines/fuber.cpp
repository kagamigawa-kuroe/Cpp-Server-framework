//
// Created by hongzhe on 22-7-18.
//

#include "fiber.h"
#include "../Log/log.h"
#include "../config/config.h"
#include "../utils/macro.h"
#include <atomic>
namespace euterpe{

    static Logger::ptr g_logger = EUTERPE_LOG_NAME("system");
    static std::atomic<uint64_t> s_fiber_id {0};
    static std::atomic<uint64_t> s_fiber_count {0};

    static thread_local Fiber* t_fiber = nullptr;
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

    void Fiber::SetThis(Fiber* f){

    };

    Fiber::Fiber(std::function<void()> cb, size_t stacksize):m_id(++s_fiber_id)
    ,m_cb(cb){
        ++s_fiber_count;
        m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
        m_stack = StackAllocator::Alloc(m_stacksize);
        if(getcontext(&m_ctx)) {
            EUTERPE_ASSERT2(false, "getcontext");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        EUTERPE_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
    };

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

    /// 抢占线程和让出线程
    void Fiber::swapIn(){
        SetThis(this);
        EUTERPE_ASSERT(m_state != EXEC);
        m_state = EXEC;

        /// swap context
    };

    void Fiber::swapOut(){

    };

    /// 返回当前协程
    Fiber::ptr Fiber::GetThis(){};


    /// 协程切换到后台并且切换状态
    void Fiber::YieldToReady(){};

    void Fiber::YieldToHold(){

    };

    /// 获取总携程数
    uint64_t Fiber::TotalFibers(){

    };

    /// 协程执行函数 执行完成返回到线程主协程
    void Fiber::MainFunc(){

    };
}