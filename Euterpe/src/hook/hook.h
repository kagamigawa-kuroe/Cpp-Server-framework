//
// Created by hongzhe on 22-8-6.
//

#ifndef EUTERPE_HOOK_H
#define EUTERPE_HOOK_H

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief hook类是用于在某些动态库函数上拓展新的功能
 * @brief 为了定制更细致化的库函数用法
 */

namespace euterpe {
    /**
     * @brief 判断当前线程是否可以启动hook
     * @return
     */
    bool is_hook_enable();

    /**
     * @brief 设置当前线程是否可以使用hook
     * @param flag
     */
    void set_hook_enable(bool flag);
}

/**
 * @brief extern C的作用在于 告诉编译器下面这一段代码是要用C
 *        语言的编译器进行编译
 */
extern "C" {
/**
 * @brief 重写两个sleep函数 sleep_f为原本的库函数 sleep_fun为函数指针类型
 */
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

/**
 * @brief 后面统一使用这个格式 funcName_f为原本的函数 funcName_fun为该函数类型的函数指针
 * @brief 后续可能会涉及到宏定义 所以这里需要统一格式
 */
typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;
}

#endif //EUTERPE_HOOK_H
