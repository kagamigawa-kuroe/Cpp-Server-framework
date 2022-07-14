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
#include <dispatch/dispatch.h>

namespace euterpe{
    class Semaphore: Noncopyable{
    public:
        Semaphore(uint32_t count = 0);
        ~Semaphore();
        void wait();
        void notify();
    private:
//        sem_t m_semaphore;
          sem_t* m_semaphore_ptr = nullptr;
    };
}


#endif //EUTERPE_MUTEX_H
