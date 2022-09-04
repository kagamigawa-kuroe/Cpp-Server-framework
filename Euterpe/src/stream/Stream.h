//
// Created by hongzhe on 22-9-3.
//

#ifndef EUTERPE_STREAM_H
#define EUTERPE_STREAM_H
#include "../ByteArray/ByteArray.h"
#include <memory>

namespace euterpe {

    class Stream {
    public:
        typedef std::shared_ptr<Stream> ptr;
        virtual ~Stream() {}

        /// 接受数据 接受到一块buffer内存中
        virtual int read(void* buffer, size_t length) = 0;

        /// 接受到一块ByteArray中
        virtual int read(ByteArray::ptr ba, size_t length) = 0;

        /// 接受固定长度的数据
        /// readFixSize会不断的循环调用上面的 read 直到读到满足的长度为止
        /// 注意 上面的read函数 不一定会读到length的长度的数据
        /// 例如 某一时刻 只有10byte的数据准备好了 但我想读的长度是20byte
        /// 那么此时就只会读到10byte
        /// 而下面这个读固定长度的作用就是 通过循环 读到需要的长度
        /// 这一举措可以有效解决一些问题 例如tcp粘包
        virtual int readFixSize(void* buffer, size_t length);

        /// 接受固定长度的数据到ByteArray中
        virtual int readFixSize(ByteArray::ptr ba, size_t length);

        virtual int write(const void* buffer, size_t length) = 0;

        virtual int write(ByteArray::ptr ba, size_t length) = 0;

        virtual int writeFixSize(const void* buffer, size_t length);

        virtual int writeFixSize(ByteArray::ptr ba, size_t length);

        /// 关闭
        virtual void close() = 0;
    };

} // euterpe

#endif //EUTERPE_STREAM_H
