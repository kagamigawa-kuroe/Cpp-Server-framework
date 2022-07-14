//
// Created by 王泓哲 on 14/07/2022.
//

#include "mutex.h"
#include <dispatch/dispatch.h>
#include <iostream>

namespace euterpe{
    Semaphore::Semaphore(uint32_t count){
        m_semaphore_ptr = sem_open("test",  O_CREAT|O_EXCL, S_IRWXU, 1);
        if(m_semaphore_ptr==nullptr) {
            throw std::logic_error("sem_init error");
        }
    };

    Semaphore::~Semaphore(){
        sem_destroy(m_semaphore_ptr);
    };

    void Semaphore::wait(){
        if(sem_wait(m_semaphore_ptr)) {
            std::cout << "111" <<std::endl;
            throw std::logic_error("sem_wait error");
        }
    };

    void Semaphore::notify(){
        if(sem_post(m_semaphore_ptr)) {
            throw std::logic_error("sem_post error");
        }
    };
}