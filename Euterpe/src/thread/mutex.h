//
// Created by 王泓哲 on 14/07/2022.
//

#ifndef EUTERPE_MUTEX_H
#define EUTERPE_MUTEX_H
#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <atomic>
#include <list>
#include "../utils/noncopyable.h"
#include <stdint.h>
#include <atomic>
#include <list>


namespace euterpe{
    class Semaphore: Noncopyable{
    public:
        Semaphore(uint32_t count = 0);
        ~Semaphore();
        void wait();
        void notify();
    private:
        sem_t m_semaphore;

    };

    /// model de lock
    /// wrapper any kind of mutex
    /// to avoid forget unlock
    template<class T>
    struct ScopedLockImpl {
    public:
        ScopedLockImpl(T& mutex)
                :m_mutex(mutex) {
            m_mutex.lock();
            m_locked = true;
        }

        ~ScopedLockImpl() {
            unlock();
        }

        void lock() {
            if(!m_locked) {
                m_mutex.lock();
                m_locked = true;
            }
        }

        void unlock() {
            if(m_locked) {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        /// mutex
        T& m_mutex;
        /// 是否已上锁
        bool m_locked;
    };

    template<class T>
    struct ReadScopedLockImpl {
    public:

        ReadScopedLockImpl(T& mutex)
                :m_mutex(mutex) {
            m_mutex.rdlock();
            m_locked = true;
        }

        ~ReadScopedLockImpl() {
            unlock();
        }

        void lock() {
            if(!m_locked) {
                m_mutex.rdlock();
                m_locked = true;
            }
        }

        void unlock() {
            if(m_locked) {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        /// mutex
        T& m_mutex;
        /// 是否已上锁
        bool m_locked;
    };

    template<class T>
    struct WriteScopedLockImpl {
    public:

        WriteScopedLockImpl(T& mutex)
                :m_mutex(mutex) {
            m_mutex.wrlock();
            m_locked = true;
        }

        ~WriteScopedLockImpl() {
            unlock();
        }

        void lock() {
            if(!m_locked) {
                m_mutex.wrlock();
                m_locked = true;
            }
        }

        void unlock() {
            if(m_locked) {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        /// Mutex
        T& m_mutex;
        /// 是否已上锁
        bool m_locked;
    };

    /// normal mutex
    class Mutex : Noncopyable {
    public:
        typedef ScopedLockImpl<Mutex> Lock;

        Mutex() {
            pthread_mutex_init(&m_mutex, nullptr);
        }

        ~Mutex() {
            pthread_mutex_destroy(&m_mutex);
        }

        void lock() {
            pthread_mutex_lock(&m_mutex);
        }

        void unlock() {
            pthread_mutex_unlock(&m_mutex);
        }
    private:
        /// wrapper a pthread_mutex_t
        pthread_mutex_t m_mutex;
    };

    class RWMutex : Noncopyable{
    public:

        /// 局部读锁
        typedef ReadScopedLockImpl<RWMutex> ReadLock;

        /// 局部写锁
        typedef WriteScopedLockImpl<RWMutex> WriteLock;

        RWMutex() {
            pthread_rwlock_init(&m_lock, nullptr);
        }

        ~RWMutex() {
            pthread_rwlock_destroy(&m_lock);
        }

        void rdlock() {
            pthread_rwlock_rdlock(&m_lock);
        }

        void wrlock() {
            pthread_rwlock_wrlock(&m_lock);
        }

        void unlock() {
            pthread_rwlock_unlock(&m_lock);
        }
    private:
        /// 读写锁
        pthread_rwlock_t m_lock;
    };

    class Spinlock : Noncopyable {
    public:
        /// 局部锁
        typedef ScopedLockImpl<Spinlock> Lock;

        Spinlock() {
            pthread_spin_init(&m_mutex, 0);
        }

        ~Spinlock() {
            pthread_spin_destroy(&m_mutex);
        }

        void lock() {
            pthread_spin_lock(&m_mutex);
        }

        void unlock() {
            pthread_spin_unlock(&m_mutex);
        }
    private:
        /// 自旋锁
        pthread_spinlock_t m_mutex;
    };

    class CASLock : Noncopyable {
    public:
        /// 局部锁
        typedef ScopedLockImpl<CASLock> Lock;

        CASLock() {
            m_mutex.clear();
        }

        ~CASLock() {
        }

        void lock() {
            /// 原子上将atomic_flag指向的状态更改obj为set（true）并返回以前的值
            while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
        }

        void unlock() {
            std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
        }
    private:
        /// 原子状态
        /// 当要求使用 volatile 声明的变量的值的时候，系统总是重新从它所在的内存读取数据
        /// 即使它前面的指令刚刚从该处读取过数据。
        /// 而且读取的数据立刻被保存。
        volatile std::atomic_flag m_mutex;
    };

}


#endif //EUTERPE_MUTEX_H
