//
// Created by hongzhe on 22-7-21.
//

#ifndef EUTERPE_SCHEDULER_H
#define EUTERPE_SCHEDULER_H

#include "../config/config.h"
#include "../thread/euterpe_thread.h"
#include "../utils/utils.h"
#include "../utils/singleton.h"
#include "../utils/noncopyable.h"
#include "../Log/log.h"
#include "../thread/euterpe_thread.h"
#include "../thread/mutex.h"
#include "../utils/macro.h"
#include "../coroutines/fiber.h"
#include <memory>
#include <functional>
#include <vector>

namespace euterpe{
    class Scheduler{
    public:
        typedef std::shared_ptr<Scheduler> ptr;
        typedef Mutex MutexType;

        /// use_caller 是否把创建构造器的线程也纳入调度器中

        /// explicit构造函数是用来防止隐式转换的。
        explicit Scheduler(size_t threads=1, bool use_caller = true, const std::string& name ="");
        virtual ~Scheduler();

        static Scheduler* GetThis();
        static Fiber* GetMainFiber();

        void start();
        void stop();

        template<class FiberOrCb>
        void schedule(FiberOrCb fc, int thread = -1) {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                need_tickle = scheduleNoLock(fc, thread);
            }

            if(need_tickle) {
                tickle();
            }
        }

        template<class InputIterator>
        void schedule(InputIterator begin, InputIterator end) {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                while(begin != end) {
                    need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                    ++begin;
                }
            }
            if(need_tickle) {
                tickle();
            }
        }

        [[nodiscard]] const std::string &getName() const;

        void setName(const std::string &name);

    private:

        template<class FiberOrCb>
        bool scheduleNoLock(FiberOrCb fc, int thread) {
            bool need_tickle = m_fibers.empty();
            FiberAndThread ft(fc, thread);
            if(ft.fiber || ft.cb) {
                m_fibers.push_back(ft);
            }
            return need_tickle;
        }

        struct FiberAndThread{
            Fiber::ptr fiber;
            std::function<void()> cb;
            int thread;
            FiberAndThread(Fiber::ptr f, int thr):fiber(f),thread(thr) { };
            FiberAndThread(Fiber::ptr* f, int thr):thread(thr){
                fiber.swap(*f);
            }
            FiberAndThread(std::function<void()> f, int thr)
                    :cb(f), thread(thr) { }
            FiberAndThread(std::function<void()>* f, int thr)
                    :thread(thr) {
                cb.swap(*f);
            }

            FiberAndThread()
                    :thread(-1) {
            }

            void reset() {
                fiber = nullptr;
                cb = nullptr;
                thread = -1;
            }

        };

    protected:
        /**
     * @brief 通知协程调度器有任务了
     */

        virtual void tickle();
        /**
         * @brief 协程调度函数
         */
        void run();

        /**
         * @brief 返回是否可以停止
         */
        virtual bool stopping();

        /**
         * @brief 协程无任务可调度时执行idle协程
         */
        virtual void idle();

        /**
         * @brief 设置当前的协程调度器
         */
        void setThis();

        /**
         * @brief 是否有空闲线程
         */
        bool hasIdleThreads() { return m_idleThreadCount > 0;}

    private:
        MutexType m_mutex;

        /// 线程池
        std::vector<Thread::ptr> m_threads;

        /// fiber队列 function or fiber
        std::vector<FiberAndThread> m_fibers;

        /// use_caller为true时有效, 调度协程
        Fiber::ptr m_rootFiber;
        std::string name;

    protected:
        /// 协程下的线程id数组
        std::vector<int> m_threadIds;
        /// 线程数量
        size_t m_threadCount = 0;
        /// 工作线程数量
        std::atomic<size_t> m_activeThreadCount = {0};
        /// 空闲线程数量
        std::atomic<size_t> m_idleThreadCount = {0};
        /// 是否正在停止
        bool m_stopping = true;
        /// 是否自动停止
        bool m_autoStop = false;
        /// 主线程id(use_caller)
        int m_rootThread = 0;

    };
}

#endif //EUTERPE_SCHEDULER_H
