//
// Created by 王泓哲 on 04/07/2022.
//

#ifndef EUTERPE_UTILS_H
#define EUTERPE_UTILS_H

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

namespace euterpe{
    pid_t GetThreadId();

    uint32_t GetFiberId();
}

#endif //EUTERPE_UTILS_H
