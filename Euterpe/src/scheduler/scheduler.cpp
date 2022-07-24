//
// Created by hongzhe on 22-7-21.
//

#include <string>
#include "scheduler.h"
#include "../Log/log.h"

namespace euterpe{
    static euterpe::Logger::ptr g_logger = EUTERPE_LOG_NAME("system");
    static thread_local Scheduler* t_scheduler = nullptr;
    static thread_local Fiber* t_scheduler_fiber = nullptr;

    void euterpe::Scheduler::setName(const std::__cxx11::basic_string<char> &name) {
        Scheduler::name = name;
    }

    const std::string &euterpe::Scheduler::getName() const {
        return name;
    }

    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name) :name(name) {
        EUTERPE_ASSERT(threads > 0);

        if (use_caller) {
            euterpe::Fiber::GetThis();
            --threads;

            EUTERPE_ASSERT(GetThis() == nullptr);
            t_scheduler = this;

            /// 创建携程调度协程
            /// run就是这个调度协程的内容
            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
            euterpe::Thread::SetName(name);

            t_scheduler_fiber = m_rootFiber.get();
            m_rootThread = euterpe::GetThreadId();
            m_threadIds.push_back(m_rootThread);
        } else {
            m_rootThread = -1;
        }
        m_threadCount = threads;
    }

    Scheduler* Scheduler::GetThis() {
        return t_scheduler;
    }

    void Scheduler::setThis() {
        t_scheduler = this;
    }

    Scheduler::~Scheduler() {
        EUTERPE_ASSERT(m_stopping);
        if(GetThis() == this) {
            t_scheduler = nullptr;
        }
    }

    Fiber* Scheduler::GetMainFiber() {
        return t_scheduler_fiber;
    }

    /// 开辟n个线程 并且在每个线程中直接执行run函数
    /// run函数的开头会调用fiber的getthis方法 用当前的状态创建一个协程调度协程
    /// 同时创建一个idle协程 用来在没有任务时空转
    void Scheduler::start() {
        MutexType::Lock lock(m_mutex);
        if(!m_stopping) {
            return;
        }
        m_stopping = false;
        EUTERPE_ASSERT(m_threads.empty());

        m_threads.resize(m_threadCount);
        for(size_t i = 0; i < m_threadCount; ++i) {
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                    , name + "_" + std::to_string(i)));
            m_threadIds.push_back(m_threads[i]->GetId());
        }
        lock.unlock();
    }

    /// wait for all fibers finish
    /// stop分两种情况 一种是scheduler用了usecaller
    /// 用了的一定要去创建scheduler的线程中执行stop
    /// 没用可以在任意线程中执行stop
    void Scheduler::stop() {
        m_autoStop = true;
        if(m_rootFiber
           && m_threadCount == 0
           && (m_rootFiber->getState() == Fiber::TERM
               || m_rootFiber->getState() == Fiber::INIT)) {
            EUTERPE_LOG_INFO(g_logger) << this << " stopped";
            m_stopping = true;

            if(stopping()) {
                return;
            }
        }

        //bool exit_on_this_fiber = false;
        if(m_rootThread != -1) {
            EUTERPE_ASSERT(GetThis() == this);
        } else {
            EUTERPE_ASSERT(GetThis() != this);
        }

        m_stopping = true;
        for(size_t i = 0; i < m_threadCount; ++i) {
            tickle();
        }

        if(m_rootFiber) {
            tickle();
        }

        if(m_rootFiber) {

            if(!stopping()) {
                m_rootFiber->call();
            }
        }

        std::vector<Thread::ptr> thrs;
        {
            MutexType::Lock lock(m_mutex);
            thrs.swap(m_threads);
        }

        for(auto& i : thrs) {
            i->join();
        }
    }

    void Scheduler::run() {
        EUTERPE_LOG_DEBUG(g_logger) << name << " run";
        /// set_hook_enable(true);

        /// 先把当前的schedule置为自己
        setThis();
        if(euterpe::GetThreadId() != m_rootThread) {
            /// 这里会创建并设置t_fiber 就是之前的当前执行携程 并且赋值给调度协程
            t_scheduler_fiber = Fiber::GetThis().get();
        }

        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
        Fiber::ptr cb_fiber;

        FiberAndThread ft;
        while(true) {
            ft.reset();
            bool tickle_me = false;
            bool is_active = false;
            {
                MutexType::Lock lock(m_mutex);
                auto it = m_fibers.begin();
                while(it != m_fibers.end()) {

                    /// 判断是否要由但前线程执行
                    if(it->thread != -1 && it->thread != euterpe::GetThreadId()) {
                        ++it;
                        tickle_me = true;
                        continue;
                    }

                    EUTERPE_ASSERT(it->fiber || it->cb);
                    if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                        ++it;
                        continue;
                    }

                    ft = *it;
                    m_fibers.erase(it++);
                    ++m_activeThreadCount;
                    is_active = true;
                    break;
                }
                tickle_me |= it != m_fibers.end();
            }

            if(tickle_me) {
                tickle();
            }

            /// 如果是一个fiber 就用swapin函数 切换到当前调度协程context执行
            if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                            && ft.fiber->getState() != Fiber::EXCEPT)) {
                /// exe
                ft.fiber->swapIn();
                --m_activeThreadCount;

                /// 如果还没有结束 就继续放入队列中
                if(ft.fiber->getState() == Fiber::READY) {
                    schedule(ft.fiber);
                } else if(ft.fiber->getState() != Fiber::TERM
                          && ft.fiber->getState() != Fiber::EXCEPT) {
                    ft.fiber->m_state = Fiber::HOLD;
                }
                /// 重置ft变量
                ft.reset();
                /// 如果是一个函数
            } else if(ft.cb) {
                /// 把这个函数放入cd_fiber中
                if(cb_fiber) {
                    cb_fiber->reset(ft.cb);
                } else {
                    cb_fiber.reset(new Fiber(ft.cb));
                }
                ft.reset();
                /// exe
                cb_fiber->swapIn();
                --m_activeThreadCount;
                /// 和之前流程相同
                if(cb_fiber->getState() == Fiber::READY) {
                    schedule(cb_fiber);
                    cb_fiber.reset();
                } else if(cb_fiber->getState() == Fiber::EXCEPT
                          || cb_fiber->getState() == Fiber::TERM) {
                    cb_fiber->reset(nullptr);
                } else {//if(cb_fiber->getState() != Fiber::TERM) {
                    cb_fiber->m_state = Fiber::HOLD;
                    cb_fiber.reset();
                }
                /// 直到某一次没有任务执行 我们去执行idle
            } else {
                if(is_active) {
                    --m_activeThreadCount;
                    continue;
                }
                if(idle_fiber->getState() == Fiber::TERM) {
                    EUTERPE_LOG_INFO(g_logger) << "idle fiber term";
                    break;
                }

                ++m_idleThreadCount;
                idle_fiber->swapIn();
                --m_idleThreadCount;
                if(idle_fiber->getState() != Fiber::TERM
                   && idle_fiber->getState() != Fiber::EXCEPT) {
                    idle_fiber->m_state = Fiber::HOLD;
                }
            }
        }
    }

    void Scheduler::tickle() {
        EUTERPE_LOG_INFO(g_logger) << "tickle";
    }

    bool Scheduler::stopping() {
        MutexType::Lock lock(m_mutex);
        return m_autoStop && m_stopping
               && m_fibers.empty() && m_activeThreadCount == 0;
    }

    void Scheduler::idle() {
        EUTERPE_LOG_INFO(g_logger) << "idle";
        while(!stopping()) {
            euterpe::Fiber::YieldToHold();
        }
    }


}
