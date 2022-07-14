//
// Created by 王泓哲 on 12/07/2022.
//

#ifndef EUTERPE_EUTERPE_THREAD_H
#define EUTERPE_EUTERPE_THREAD_H

#include <thread>
#include <functional>
#include <memory>
#include <sys/types.h>
#include <boost/noncopyable.hpp>
#include "../utils/noncopyable.h"
#include "mutex.h"

namespace euterpe{

    class Thread: Noncopyable{
    public:
        typedef std::shared_ptr<Thread> ptr;
        Thread(std::function<void()> cb,const std::string& name);
        Thread() = default;
        void test();
        ~Thread();
        const std::string& getName() const { return m_name; }
        void join();
        static Thread* GetThis();
        static const std::string& GetName();
        static void SetName(const std::string& name);

    private:
        static void* run(void* arg);

    private:
        /// 注意pid_t和pthread_t的区别
        pid_t m_id = -1;
        /// 线程ID
        pthread_t m_thread = 0;
        /// 线程执行函数
        std::function<void()> m_cb;
        std::string m_name;
        /// 信号量
        Semaphore m_semaphore;

    };
}

#endif //EUTERPE_EUTERPE_THREAD_H
