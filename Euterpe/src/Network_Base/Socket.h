//
// Created by hongzhe on 22-8-20.
//

#ifndef EUTERPE_SOCKET_H
#define EUTERPE_SOCKET_H

#include <memory>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "address.h"
#include "../utils/noncopyable.h"

namespace euterpe {

    /**
     * @brief Socket封装类
     */
    class Socket : public std::enable_shared_from_this<Socket>, Noncopyable {
    public:
        typedef std::shared_ptr<Socket> ptr;
        typedef std::weak_ptr<Socket> weak_ptr;

        enum Type {
            /// TCP类型
            TCP = SOCK_STREAM,
            /// UDP类型
            UDP = SOCK_DGRAM
        };

        enum Family {
            /// IPv4 socket
            IPv4 = AF_INET,
            /// IPv6 socket
            IPv6 = AF_INET6,
            /// Unix socket
            UNIX = AF_UNIX,
        };

        /// 创建TCP Socket(满足地址类型)
        static Socket::ptr CreateTCP(euterpe::Address::ptr address);

        /// 创建UDP Socket(满足地址类型)
        static Socket::ptr CreateUDP(euterpe::Address::ptr address);

        /// 创建IPv4的TCP Socket
        static Socket::ptr CreateTCPSocket();

        /// 创建IPv4的UDP Socket
        static Socket::ptr CreateUDPSocket();

        /// 创建IPv6的TCP Socket
        static Socket::ptr CreateTCPSocket6();

        /// 创建IPv6的UDP Socket
        static Socket::ptr CreateUDPSocket6();

        /// 创建Unix的TCP Socket
        static Socket::ptr CreateUnixTCPSocket();

        /// 创建Unix的UDP Socket
        static Socket::ptr CreateUnixUDPSocket();

        /// 构造函数
        Socket(int family, int type, int protocol = 0);

        /// 析构函数
        virtual ~Socket();

        /// 获取发送超时时间(毫秒)
        int64_t getSendTimeout();

        /// 设置发送超时时间(毫秒)
        void setSendTimeout(int64_t v);

        /// 获取接受超时时间(毫秒)
        int64_t getRecvTimeout();

        /// 设置接受超时时间(毫秒)
        void setRecvTimeout(int64_t v);

        /// 获取sockopt
        bool getOption(int level, int option, void* result, socklen_t* len);

        template<class T>
        bool getOption(int level, int option, T& result) {
            socklen_t length = sizeof(T);
            return getOption(level, option, &result, &length);
        }

        /// 设置sockopt
        bool setOption(int level, int option, const void* result, socklen_t len);

        template<class T>
        bool setOption(int level, int option, const T& value) {
            return setOption(level, option, &value, sizeof(T));
        }

        virtual Socket::ptr accept();

        virtual bool bind(const Address::ptr addr);

        virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

        virtual bool reconnect(uint64_t timeout_ms = -1);

        virtual bool listen(int backlog = SOMAXCONN);

        virtual bool close();

        virtual int send(const void* buffer, size_t length, int flags = 0);

        virtual int send(const iovec* buffers, size_t length, int flags = 0);

        virtual int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);

        virtual int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);

        virtual int recv(void* buffer, size_t length, int flags = 0);

        virtual int recv(iovec* buffers, size_t length, int flags = 0);

        virtual int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);

        virtual int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

        /// 对段地址
        Address::ptr getRemoteAddress();

        /// 自己的地址
        Address::ptr getLocalAddress();


        int getFamily() const { return m_family;}

        int getType() const { return m_type;}

        int getProtocol() const { return m_protocol;}

        bool isConnected() const { return m_isConnected;}

        bool isValid() const;

        int getError();

        /**
         * @brief 输出信息到流中
         */
        virtual std::ostream& dump(std::ostream& os) const;

        virtual std::string toString() const;

        /**
         * @brief 返回socket句柄
         */
        int getSocket() const { return m_sock;}

        /**
         * @brief 取消读
         */
        bool cancelRead();

        /**
         * @brief 取消写
         */
        bool cancelWrite();

        /**
         * @brief 取消accept
         */
        bool cancelAccept();

        /**
         * @brief 取消所有事件
         */
        bool cancelAll();
    protected:
        /**
         * @brief 初始化socket
         */
        void initSock();

        /**
         * @brief 创建socket
         */
        void newSock();

        /**
         * @brief 初始化sock
         */
        virtual bool init(int sock);
    protected:
        /// socket句柄
        int m_sock;
        /// 协议簇
        int m_family;
        /// 类型
        int m_type;
        /// 协议
        int m_protocol;
        /// 是否连接
        bool m_isConnected;
        /// 本地地址
        Address::ptr m_localAddress;
        /// 远端地址
        Address::ptr m_remoteAddress;
    };
    std::ostream& operator<<(std::ostream& os, const Socket& sock);
} // euterpe

#endif //EUTERPE_SOCKET_H
