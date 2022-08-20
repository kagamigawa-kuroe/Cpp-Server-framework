//
// Created by hongzhe on 22-8-19.
//

#ifndef EUTERPE_ENDIAN_H
#define EUTERPE_ENDIAN_H

/// 小端为1 大端为2
#define EUTERPE_LITTLE_ENDIAN 1
#define EUTERPE_BIG_ENDIAN 2

/// byteswap库用于大小端转换
#include <byteswap.h>
#include <cstdint>
#include <type_traits>
/// endian.h库中 BYTE_ORDER宏 可以用来判断大小端
#include <bits/endian.h>

namespace euterpe {

    template<class T>
    typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
    byteswap(T value) {
        return (T) bswap_64((uint64_t) value);
    }

    /**
     * @brief 4字节类型的字节序转化
     */
     /// enable_if 元模板用法 只有当判断条件为true T类型才有效 否则产生编译错误
     /// ::type返回T类型
     /// bswap_32/bswap_64用于大小端的转换
     /// 大端方式将高位存放在低地址(人类正常用法)，小端方式将低位存放在高地址
    template<class T>
    typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
    byteswap(T value) {
        /// 转换32位大小端
        return (T) bswap_32((uint32_t) value);
    }

    /**
     * @brief 2字节类型的字节序转化
     */
    template<class T>
    typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
    byteswap(T value) {
        /// 转换16位大小端
        return (T) bswap_16((uint16_t) value);
    }

#if BYTE_ORDER == BIG_ENDIAN
#define EUTERPE_BYTE_ORDER EUTERPE_BIG_ENDIAN
#else
#define EUTERPE_BYTE_ORDER EUTERPE_LITTLE_ENDIAN
#endif

#if EUTERPE_BYTE_ORDER == EUTERPE_BIG_ENDIAN

    /**
     * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
     */
    template<class T>
    T byteswapOnLittleEndian(T t) {
        return t;
    }

    /**
     * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
     */
    template<class T>
    T byteswapOnBigEndian(T t) {
        return byteswap(t);
    }

#else

    /**
    * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
    */
    template<class T>
    T byteswapOnLittleEndian(T t) {
    return byteswap(t);
    }

    /**
    * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
    */
    template<class T>
    T byteswapOnBigEndian(T t) {
    return t;
    }
#endif

} // euterpe

#endif //EUTERPE_ENDIAN_H
