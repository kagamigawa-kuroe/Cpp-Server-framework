//
// Created by hongzhe on 22-8-20.
//

#include "Socket.h"
#include "../IO/IoManager.h"
#include "../fd_manager/fd_manager.h"
#include "../Log/log.h"
#include "../utils/macro.h"
#include "../hook/hook.h"
#include <limits.h>


namespace euterpe {
    static euterpe::Logger::ptr g_logger = EUTERPE_LOG_NAME("system");

    /// 一系列方法 直接返回一个指向一个Socket类的只能指针
    /// 注意这里的Socket类只是我们自己的Socket封装类
    /// 此时还并没有调用socket系统函数创造套接字 只是将协议之类的成员函数传进去了
    Socket::ptr Socket::CreateTCP(euterpe::Address::ptr address) {
        Socket::ptr sock(new Socket(address->getFamily(), TCP, 0));
        return sock;
    }

    Socket::ptr Socket::CreateUDP(euterpe::Address::ptr address) {
        Socket::ptr sock(new Socket(address->getFamily(), UDP, 0));
        sock->newSock();
        sock->m_isConnected = true;
        return sock;
    }

    Socket::ptr Socket::CreateTCPSocket() {
        Socket::ptr sock(new Socket(IPv4, TCP, 0));
        return sock;
    }

    Socket::ptr Socket::CreateUDPSocket() {
        Socket::ptr sock(new Socket(IPv4, UDP, 0));
        sock->newSock();
        sock->m_isConnected = true;
        return sock;
    }

    Socket::ptr Socket::CreateTCPSocket6() {
        Socket::ptr sock(new Socket(IPv6, TCP, 0));
        return sock;
    }

    Socket::ptr Socket::CreateUDPSocket6() {
        Socket::ptr sock(new Socket(IPv6, UDP, 0));
        sock->newSock();
        sock->m_isConnected = true;
        return sock;
    }

    Socket::ptr Socket::CreateUnixTCPSocket() {
        Socket::ptr sock(new Socket(UNIX, TCP, 0));
        return sock;
    }

    Socket::ptr Socket::CreateUnixUDPSocket() {
        Socket::ptr sock(new Socket(UNIX, UDP, 0));
        return sock;
    }

    Socket::Socket(int family, int type, int protocol)
            :m_sock(-1)
            ,m_family(family)
            ,m_type(type)
            ,m_protocol(protocol)
            ,m_isConnected(false) {
    }

    Socket::~Socket() {
        close();
    }

    /// 先去FdMgr中拿到文件描述符 然后获取其过期事件
    int64_t Socket::getSendTimeout() {
        FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
        if(ctx) {
            return ctx->getTimeout(SO_SNDTIMEO);
        }
        return -1;
    }

    /// 设置过期事件
    /// setOption 底层是 setsockopt 在hook中已经被重写过了
    void Socket::setSendTimeout(int64_t v) {
        struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
        setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
    }

    int64_t Socket::getRecvTimeout() {
        FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
        if(ctx) {
            return ctx->getTimeout(SO_RCVTIMEO);
        }
        return -1;
    }

    void Socket::setRecvTimeout(int64_t v) {
        struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
        /// SOL_SOCKET代表当前的socket
        setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
    }

    bool Socket::getOption(int level, int option, void* result, socklen_t* len) {
        int rt = getsockopt(m_sock, level, option, result, (socklen_t*)len);
        if(rt) {
            EUTERPE_LOG_DEBUG(g_logger) << "getOption sock=" << m_sock
                                      << " level=" << level << " option=" << option
                                      << " errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }
        return true;
    }

    bool Socket::setOption(int level, int option, const void* result, socklen_t len) {
        if(setsockopt(m_sock, level, option, result, (socklen_t)len)) {
            EUTERPE_LOG_DEBUG(g_logger) << "setOption sock=" << m_sock
                                      << " level=" << level << " option=" << option
                                      << " errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }
        return true;
    }

    /// 接受 然后新创一个Socket返回
    Socket::ptr Socket::accept() {
        Socket::ptr sock(new Socket(m_family, m_type, m_protocol));
        int newsock = ::accept(m_sock, nullptr, nullptr);
        if(newsock == -1) {
            EUTERPE_LOG_ERROR(g_logger) << "accept(" << m_sock << ") errno="
                                      << errno << " errstr=" << strerror(errno);
            return nullptr;
        }
        if(sock->init(newsock)) {
            return sock;
        }
        return nullptr;
    }

    /// 通过一个socket描述符 初始化一个Socket类
    bool Socket::init(int sock) {
        FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock);
        if(ctx && ctx->isSocket() && !ctx->isClose()) {
            m_sock = sock;
            m_isConnected = true;
            initSock();
            getLocalAddress();
            getRemoteAddress();
            return true;
        }
        return false;
    }

    bool Socket::bind(const Address::ptr addr) {
        /// 如果该Socket类封装的文件描述符非法
        /// 重新初始化
        if(!isValid()) {
            newSock();
            /// 如果无法成功初始化 则报错
            if(EUTERPE_UNLIKELY(!isValid())) {
                return false;
            }
        }

        if(EUTERPE_UNLIKELY(addr->getFamily() != m_family)) {
            EUTERPE_LOG_ERROR(g_logger) << "bind sock.family("
                                      << m_family << ") addr.family(" << addr->getFamily()
                                      << ") not equal, addr=" << addr->toString();
            return false;
        }

        UnixAddress::ptr uaddr = std::dynamic_pointer_cast<UnixAddress>(addr);
        if(uaddr) {
            Socket::ptr sock = Socket::CreateUnixTCPSocket();
            if(sock->connect(uaddr)) {
                return false;
            } else {
                euterpe::FSUtil::Unlink(uaddr->getPath(), true);
            }
        }

        /// 调用经过hook修改之后的bind函数
        if(::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
            EUTERPE_LOG_ERROR(g_logger) << "bind error errrno=" << errno
                                      << " errstr=" << strerror(errno);
            return false;
        }
        getLocalAddress();
        return true;
    }

    bool Socket::reconnect(uint64_t timeout_ms) {
        if(!m_remoteAddress) {
            EUTERPE_LOG_ERROR(g_logger) << "reconnect m_remoteAddress is null";
            return false;
        }
        m_localAddress.reset();
        return connect(m_remoteAddress, timeout_ms);
    }

    bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms) {
        m_remoteAddress = addr;
        if(!isValid()) {
            newSock();
            if(EUTERPE_UNLIKELY(!isValid())) {
                return false;
            }
        }

        if(EUTERPE_UNLIKELY(addr->getFamily() != m_family)) {
            EUTERPE_LOG_ERROR(g_logger) << "connect sock.family("
                                      << m_family << ") addr.family(" << addr->getFamily()
                                      << ") not equal, addr=" << addr->toString();
            return false;
        }

        if(timeout_ms == (uint64_t)-1) {
            if(::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
                EUTERPE_LOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                                          << ") error errno=" << errno << " errstr=" << strerror(errno);
                close();
                return false;
            }
        } else {
            if(::connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(), timeout_ms)) {
                EUTERPE_LOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                                          << ") timeout=" << timeout_ms << " error errno="
                                          << errno << " errstr=" << strerror(errno);
                close();
                return false;
            }
        }

        /// 建立连接之后 储存双方地址
        m_isConnected = true;
        getRemoteAddress();
        getLocalAddress();
        return true;
    }

    bool Socket::listen(int backlog) {
        if(!isValid()) {
            EUTERPE_LOG_ERROR(g_logger) << "listen error sock=-1";
            return false;
        }
        if(::listen(m_sock, backlog)) {
            EUTERPE_LOG_ERROR(g_logger) << "listen error errno=" << errno
                                      << " errstr=" << strerror(errno);
            return false;
        }
        return true;
    }

    bool Socket::close() {
        if(!m_isConnected && m_sock == -1) {
            return true;
        }
        m_isConnected = false;
        if(m_sock != -1) {
            ::close(m_sock);
            m_sock = -1;
        }
        return false;
    }

    int Socket::send(const void* buffer, size_t length, int flags) {
        if(isConnected()) {
            return ::send(m_sock, buffer, length, flags);
        }
        return -1;
    }

    // msghdr和iovec 用于向多段缓冲区写入或者发送数据
    // iovec可以看作是一段缓存区域 其中有起始地址以及长度
    // msghdr可以看作是一个iovec数组 记录了第一个iovec的地址 以及总iovec个数
    // 相当于msdhdr记录了n段不同的缓冲区
    int Socket::send(const iovec* buffers, size_t length, int flags) {
        if(isConnected()) {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec*)buffers;
            msg.msg_iovlen = length;
            return ::sendmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    int Socket::sendTo(const void* buffer, size_t length, const Address::ptr to, int flags) {
        if(isConnected()) {
            return ::sendto(m_sock, buffer, length, flags, to->getAddr(), to->getAddrLen());
        }
        return -1;
    }

    int Socket::sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags) {
        if(isConnected()) {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec*)buffers;
            msg.msg_iovlen = length;
            msg.msg_name = to->getAddr();
            msg.msg_namelen = to->getAddrLen();
            return ::sendmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    int Socket::recv(void* buffer, size_t length, int flags) {
        if(isConnected()) {
            return ::recv(m_sock, buffer, length, flags);
        }
        return -1;
    }

    int Socket::recv(iovec* buffers, size_t length, int flags) {
        if(isConnected()) {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec*)buffers;
            msg.msg_iovlen = length;
            return ::recvmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    int Socket::recvFrom(void* buffer, size_t length, Address::ptr from, int flags) {
        if(isConnected()) {
            socklen_t len = from->getAddrLen();
            return ::recvfrom(m_sock, buffer, length, flags, from->getAddr(), &len);
        }
        return -1;
    }

    int Socket::recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags) {
        if(isConnected()) {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec*)buffers;
            msg.msg_iovlen = length;
            msg.msg_name = from->getAddr();
            msg.msg_namelen = from->getAddrLen();
            return ::recvmsg(m_sock, &msg, flags);
        }
        return -1;
    }


    /// getsocketname getpeername 两个核心函数 在建立连接后
    /// 用于获取自己的ip和端口 以及对方的ip和端口
    Address::ptr Socket::getRemoteAddress() {
        if(m_remoteAddress) {
            return m_remoteAddress;
        }

        Address::ptr result;
        switch(m_family) {
            case AF_INET:
                result.reset(new IPv4Address());
                break;
            case AF_INET6:
                result.reset(new IPv6Address());
                break;
            case AF_UNIX:
                result.reset(new UnixAddress());
                break;
            default:
                result.reset(new UnknownAddress(m_family));
                break;
        }
        socklen_t addrlen = result->getAddrLen();
        if(getpeername(m_sock, result->getAddr(), &addrlen)) {
            //EUTERPE_LOG_ERROR(g_logger) << "getpeername error sock=" << m_sock
            //    << " errno=" << errno << " errstr=" << strerror(errno);
            return Address::ptr(new UnknownAddress(m_family));
        }
        if(m_family == AF_UNIX) {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
            addr->setAddrLen(addrlen);
        }
        m_remoteAddress = result;
        return m_remoteAddress;
    }

    Address::ptr Socket::getLocalAddress() {
        if(m_localAddress) {
            return m_localAddress;
        }

        Address::ptr result;
        switch(m_family) {
            case AF_INET:
                result.reset(new IPv4Address());
                break;
            case AF_INET6:
                result.reset(new IPv6Address());
                break;
            case AF_UNIX:
                result.reset(new UnixAddress());
                break;
            default:
                result.reset(new UnknownAddress(m_family));
                break;
        }
        socklen_t addrlen = result->getAddrLen();
        if(getsockname(m_sock, result->getAddr(), &addrlen)) {
            EUTERPE_LOG_ERROR(g_logger) << "getsockname error sock=" << m_sock
                                      << " errno=" << errno << " errstr=" << strerror(errno);
            return Address::ptr(new UnknownAddress(m_family));
        }
        if(m_family == AF_UNIX) {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
            addr->setAddrLen(addrlen);
        }
        m_localAddress = result;
        return m_localAddress;
    }

    bool Socket::isValid() const {
        return m_sock != -1;
    }

    int Socket::getError() {
        int error = 0;
        socklen_t len = sizeof(error);
        if(!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
            error = errno;
        }
        return error;
    }

    std::ostream& Socket::dump(std::ostream& os) const {
        os << "[Socket sock=" << m_sock
           << " is_connected=" << m_isConnected
           << " family=" << m_family
           << " type=" << m_type
           << " protocol=" << m_protocol;
        if(m_localAddress) {
            os << " local_address=" << m_localAddress->toString();
        }
        if(m_remoteAddress) {
            os << " remote_address=" << m_remoteAddress->toString();
        }
        os << "]";
        return os;
    }

    std::string Socket::toString() const {
        std::stringstream ss;
        dump(ss);
        return ss.str();
    }

    bool Socket::cancelRead() {
        return IOManager::GetThis()->cancelEvent(m_sock, euterpe::IOManager::READ);
    }

    bool Socket::cancelWrite() {
        return IOManager::GetThis()->cancelEvent(m_sock, euterpe::IOManager::WRITE);
    }

    bool Socket::cancelAccept() {
        return IOManager::GetThis()->cancelEvent(m_sock, euterpe::IOManager::READ);
    }

    bool Socket::cancelAll() {
        return IOManager::GetThis()->cancelAll(m_sock);
    }

    void Socket::initSock() {
        int val = 1;
        setOption(SOL_SOCKET, SO_REUSEADDR, val);
        if(m_type == SOCK_STREAM) {
            setOption(IPPROTO_TCP, TCP_NODELAY, val);
        }
    }

    /// 通过内部的参数 初始化成员变量套接字
    void Socket::newSock() {
        m_sock = socket(m_family, m_type, m_protocol);
        if(EUTERPE_LIKELY(m_sock != -1)) {
            initSock();
        } else {
            EUTERPE_LOG_ERROR(g_logger) << "socket(" << m_family
                                      << ", " << m_type << ", " << m_protocol << ") errno="
                                      << errno << " errstr=" << strerror(errno);
        }
    }

    std::ostream& operator<<(std::ostream& os, const Socket& sock) {
        return sock.dump(os);
    }
} // euterpe