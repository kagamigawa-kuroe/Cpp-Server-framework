//
// Created by 王泓哲 on 04/07/2022.
//

#include "utils.h"
#include <sys/syscall.h>
#include <unistd.h>

namespace euterpe{
    pid_t GetThreadId(){
        return syscall(SYS_gettid);
    };
    uint32_t GetFiberId(){
        return 0;
    };
}