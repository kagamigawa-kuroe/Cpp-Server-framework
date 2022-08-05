//
// Created by hongzhe on 22-8-4.
//

#ifndef EUTERPE_TIMER_H
#define EUTERPE_TIMER_H
#include <memory>
#include <functional>
#include <set>
#include "../thread/euterpe_thread.h"

/// 定时器可以在IOManager的epoll_wait中使用 我们可以在有任务需要执行之前都陷入沉睡
namespace euterpe{

    class TimerManager;

    class Timer: public std::enable_shared_from_this<Timer>{
    public:
        friend class TimerManager;
        typedef std::shared_ptr<Timer> ptr;
    private:
        /// 时间 回调函数 是否循环 manager
        Timer(uint64_t ms,std::function<void()> cb,bool recurring,TimerManager* manager);
        /// 用于处理过期定时器的方法
        Timer(uint64_t next);

    public:
        /**
         * @brief 取消定时器
         */
        bool cancel();

        /**
         * @brief 刷新设置定时器的执行时间
         */
        bool refresh();

        /**
         * @brief 重置定时器时间
         * @param[in] ms 定时器执行间隔时间(毫秒)
         * @param[in] from_now 是否从当前时间开始计算
         */
        bool reset(uint64_t ms, bool from_now);

    private:
        /// 是否循环定时器
        bool m_recurring = false;
        /// 执行周期
        uint64_t m_ms = 0;
        /// 精确的执行时间
        uint64_t m_next = 0;
        /// 回调函数
        std::function<void()> m_cb;
        /// 定时器管理器
        TimerManager* m_manager = nullptr;

    private:
        /// 定时器比较用的仿函数
        struct Comparator {
            bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
        };

    };

    class TimerManager : public std::enable_shared_from_this<TimerManager>{
    public:
        friend class Timer;
        typedef std::shared_ptr<TimerManager> ptr;
        typedef RWMutex RWMutexType;

    public:
        TimerManager();
        virtual ~TimerManager();

        /// 添加一个普通定时器
        Timer::ptr addTimer(uint64_t ms, std::function<void()> cb,bool recurring = false);

        /// 条件触发器
        Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb
                ,std::weak_ptr<void> weak_cond
                ,bool recurring = false);

        /// 返回最近的一个定时器
        uint64_t getNextTimer();

        /// 处理过期任务
        void listExpiredCb(std::vector<std::function<void()> >& cbs);

        /// 检测服务器时间是否被调后了
        bool detectClockRollover(uint64_t now_ms);

        /// 判断有无任务
        bool hasTimer();
    protected:


        /// 当有新的定时器插入到定时器的首部,执行该函数
        /// 比如epoll_wait需要重新计算沉睡时间
        virtual void onTimerInsertedAtFront() = 0;

        /**
         * @brief 将定时器添加到管理器中
         */
        void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);

    private:
        RWMutexType m_mutex;
        /// 定时器集合
        std::set<Timer::ptr, Timer::Comparator> m_timers;
        /// 是否触发onTimerInsertedAtFront
        bool m_tickled = false;
        /// 上次执行时间
        uint64_t m_previouseTime = 0;
    };
}


#endif //EUTERPE_TIMER_H
