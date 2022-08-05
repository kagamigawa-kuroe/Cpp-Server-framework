//
// Created by hongzhe on 22-8-4.
//

#include "Timer.h"
#include "../utils/utils.h"

euterpe::Timer::Timer(uint64_t ms, std::function<void()> cb,
                      bool recurring, TimerManager* manager)
        :m_recurring(recurring)
        ,m_ms(ms)
        ,m_cb(cb)
        ,m_manager(manager) {
    m_next = euterpe::GetCurrentMS() + m_ms;
}

euterpe::Timer::Timer(uint64_t next)
        :m_next(next) {
}

bool euterpe::Timer::cancel() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    /// 把当前的定时器从manager中删除
    if(m_cb) {
        m_cb = nullptr;
        auto it = m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}

bool euterpe::Timer::refresh() {
    /// 重置他的执行时间 为 当前时间+等待时间
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb) {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);
    m_next = euterpe::GetCurrentMS() + m_ms;
    m_manager->m_timers.insert(shared_from_this());
    return true;
}

bool euterpe::Timer::reset(uint64_t ms, bool from_now) {
    if(ms == m_ms && !from_now) {
        return true;
    }
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb) {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);
    uint64_t start = 0;
    if(from_now) {
        start = euterpe::GetCurrentMS();
    } else {
        start = m_next - m_ms;
    }
    m_ms = ms;
    m_next = start + m_ms;
    m_manager->addTimer(shared_from_this(), lock);
    return true;

}

/// timer比较类函数
bool euterpe::Timer::Comparator::operator()(const euterpe::Timer::ptr &lhs, const euterpe::Timer::ptr &rhs) const {
    if(!lhs && !rhs) {
        return false;
    }
    if(!lhs) {
        return true;
    }
    if(!rhs) {
        return false;
    }
    if(lhs->m_next < rhs->m_next) {
        return true;
    }
    if(rhs->m_next < lhs->m_next) {
        return false;
    }
    return lhs.get() < rhs.get();
}

euterpe::TimerManager::TimerManager() {
    m_previouseTime = euterpe::GetCurrentMS();
}

euterpe::TimerManager::~TimerManager() {

}

euterpe::Timer::ptr euterpe::TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring) {
    Timer::ptr timer(new Timer(ms, cb, recurring, this));
    RWMutexType::WriteLock lock(m_mutex);
    addTimer(timer, lock);
    return timer;
}

void euterpe::TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock& lock) {
    auto it = m_timers.insert(val).first;
    bool at_front = (it == m_timers.begin()) && !m_tickled;
    if(at_front) {
        m_tickled = true;
    }
    lock.unlock();

    if(at_front) {
        onTimerInsertedAtFront();
    }
}

///用于判断一个定时器是否还存在
static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
    /// lock返回智能指针
    /// 如果已经被释放了 则为空
    std::shared_ptr<void> tmp = weak_cond.lock();
    if(tmp) {
        cb();
    }
}

euterpe::Timer::ptr
euterpe::TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond,
                                         bool recurring) {
    return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
}

/// 获取下一个即将执行的定时器
uint64_t euterpe::TimerManager::getNextTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    m_tickled = false;
    if(m_timers.empty()) {
        return ~0ull;
    }

    const Timer::ptr& next = *m_timers.begin();
    uint64_t now_ms = euterpe::GetCurrentMS();
    if(now_ms >= next->m_next) {
        return 0;
    } else {
        return next->m_next - now_ms;
    }
}

/// 获取需要执行的定时器的回调函数列表
void euterpe::TimerManager::listExpiredCb(std::vector<std::function<void()> >& cbs) {
    /// 获取当前时间
    uint64_t now_ms = euterpe::GetCurrentMS();
    /// 过期定时器数组
    std::vector<Timer::ptr> expired;
    {
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty()) {
            return;
        }
    }
    RWMutexType::WriteLock lock(m_mutex);
    if(m_timers.empty()) {
        return;
    }
    /// 检测服务器时间变化
    bool rollover = detectClockRollover(now_ms);
    if(!rollover && ((*m_timers.begin())->m_next > now_ms)) {
        return;
    }

    /// 用当前时间构造一个Timer
    /// 将下次执行时间设置为当前时间
    Timer::ptr now_timer(new Timer(now_ms));

    /// 找到set中这个时间之前的所有内容
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);
    while(it != m_timers.end() && (*it)->m_next == now_ms) {
        ++it;
    }

    /// 全部加入到过期集合中
    expired.insert(expired.begin(), m_timers.begin(), it);
    /// 在任务调度器中删除这些定时器
    m_timers.erase(m_timers.begin(), it);
    cbs.reserve(expired.size());

    for(auto& timer : expired) {
        cbs.push_back(timer->m_cb);
        /// 如果是循环任务 再次加入定时器列表中
        /// 如果不是 就结束
        if(timer->m_recurring) {
            timer->m_next = now_ms + timer->m_ms;
            m_timers.insert(timer);
        } else {
            timer->m_cb = nullptr;
        }
    }
}

/// 检测服务器时间是否被更改
bool euterpe::TimerManager::detectClockRollover(uint64_t now_ms) {
    bool rollover = false;
    if(now_ms < m_previouseTime &&
       now_ms < (m_previouseTime - 60 * 60 * 1000)) {
        rollover = true;
    }
    m_previouseTime = now_ms;
    return rollover;
}

/// 判断一个manager是否有定时器
bool euterpe::TimerManager::hasTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    return !m_timers.empty();
}