//
// Created by hongzhe on 22-7-17.
//

#ifndef EUTERPE_MACRO_H
#define EUTERPE_MACRO_H

#include <string.h>
#include <assert.h>
#include "../Log/log.h"
#include "utils.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define EUTERPE_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define EUTERPE_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define EUTERPE_LIKELY(x)      (x)
#   define EUTERPE_UNLIKELY(x)      (x)
#endif

/// 断言宏封装
#define EUTERPE_ASSERT(x) \
    if(EUTERPE_UNLIKELY(!(x))) { \
        EUTERPE_LOG_ERROR(EUTERPE_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << euterpe::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

/// 断言宏封装
#define EUTERPE_ASSERT2(x, w) \
    if(EUTERPE_UNLIKELY(!(x))) { \
        EUTERPE_LOG_ERROR(EUTERPE_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << euterpe::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif
