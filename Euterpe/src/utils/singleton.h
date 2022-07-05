//
// Created by 王泓哲 on 05/07/2022.
//

#ifndef EUTERPE_SINGLETON_H
#define EUTERPE_SINGLETON_H
#include <memory>

namespace euterpe {

    template<class T>
            class Singleton {
            public:
                /**
                 * @brief 返回单例裸指针
                 */
                static T* GetInstance() {
                    static T v;
                    return &v;
                }
            };

    template<class T>
            class SingletonPtr {
            public:
                /**
                 * @brief 返回单例智能指针
                 */
                static std::shared_ptr<T> GetInstance() {
                    static std::shared_ptr<T> v(new T);
                    return v;
                }
            };

}

#endif //EUTERPE_SINGLETON_H
