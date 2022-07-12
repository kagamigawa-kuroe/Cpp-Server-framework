//
// Created by 王泓哲 on 12/07/2022.
//

#ifndef EUTERPE_NONCOPYABLE_H
#define EUTERPE_NONCOPYABLE_H

namespace euterpe{
    class Noncopyable {
    public:
        /**
            * @brief 默认构造函数
        */
        Noncopyable() = default;

        /**
         * @brief 默认析构函数
         */
        ~Noncopyable() = default;

        /**
         * @brief 拷贝构造函数(禁用)
         */
        Noncopyable(const Noncopyable&) = delete;

        /**
         * @brief 赋值函数(禁用)
         */
        Noncopyable& operator=(const Noncopyable&) = delete;
    };
}


#endif //EUTERPE_NONCOPYABLE_H
