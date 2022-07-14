//
// Created by 王泓哲 on 04/07/2022.
//

#include "utils.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>

namespace euterpe{
    pid_t GetThreadId(){
        /// 只在linux环境下可用
        /// pid_t tid = syscall(SYS_gettid);

        /// 由于测试环境在mac 使用pthread替代
        uint64_t tid;
        pthread_threadid_np(NULL, &tid);
        return tid;
    };
    uint32_t GetFiberId(){
        return 0;
    };
}