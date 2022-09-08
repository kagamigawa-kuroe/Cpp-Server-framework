//
// Created by hongzhe on 22-9-2.
//

#include "tcp_server.h"
#include <memory>
#include <functional>
#include <vector>
#include "../Log/log.h"
#include "../IO/IoManager.h"
#include "../Network_Base/Socket.h"
#include "../Network_Base/address.h"
#include "../config/config.h"

namespace euterpe {

    static euterpe::Logger::ptr g_logger = EUTERPE_LOG_NAME("system");
    static ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout =
            euterpe::Config::Lookup("tcp_server.read_timeout",(uint64_t)(60 * 1000 * 2));

    ///  m_acceptWorker 专门用于处理新的连接
    TcpServer::TcpServer(IOManager *worker,IOManager* io_worker,IOManager* accept_worker):
             m_worker(worker)
            ,m_ioWorker(io_worker)
            ,m_acceptWorker(accept_worker)
            ,m_recvTimeout(g_tcp_server_read_timeout->getValue())
            ,m_name("euterpe/1.0.0")
            ,m_isStop(true) {
    }

    TcpServer::~TcpServer() {
        for(auto& i : m_socks) {
            i->close();
        }
        m_socks.clear();
    }

    bool TcpServer::bind(Address::ptr addr){
        std::vector<Address::ptr> addrs;
        std::vector<Address::ptr> fails;
        addrs.push_back(addr);
        return bind(addrs,fails);
    }

    bool TcpServer::bind(const std::vector<Address::ptr> addrs, std::vector<Address::ptr>& fails) {
        for(auto &i : addrs){
            Socket::ptr sock = Socket::CreateTCP(i);
            if(!sock->bind(i)){
                EUTERPE_LOG_ERROR(g_logger) << "bind fail errno="
                                            << errno << " errstr=" << strerror(errno)
                                            << " addr=[" << i->toString() << "]";
                fails.push_back(i);
                continue;
            }

            if(!sock->listen()){
                EUTERPE_LOG_ERROR(g_logger) << "listen fail errno="
                                          << errno << " errstr=" << strerror(errno)
                                          << " addr=[" << i->toString() << "]";
                fails.push_back(i);
                continue;
            }

            m_socks.push_back(sock);
        }

        if(!fails.empty()) {
            m_socks.clear();
            return false;
        }

        for(auto& i : m_socks) {
            EUTERPE_LOG_INFO(g_logger) << "type=" << m_type
                                     << " name=" << m_name
                                     << " server bind success: " << *i;
        }

        return true;
    }

    /// 开始时 先让m_acceptWorker调度监听所有的listensocket 地址 也就是依次执行startAccept函数
    bool TcpServer::start() {
        if(!m_isStop) {
            return true;
        }
        m_isStop = false;
        for(auto& sock : m_socks) {
            m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                                               shared_from_this(), sock));
        }
        return true;
    }

    bool TcpServer::stop() {
        m_isStop = true;
        auto self = shared_from_this();
        m_acceptWorker->schedule([this, self]() {
            for(auto& sock : m_socks) {
                sock->cancelAll();
                sock->close();
            }
            m_socks.clear();
        });
    }

    void TcpServer::handleClient(Socket::ptr client) {
        EUTERPE_LOG_INFO(g_logger) << "handleClient: " << *client;
    }

    void TcpServer::startAccept(Socket::ptr sock) {
        while(!m_isStop) {
            /// 注意这里的accept已经被hook了
            Socket::ptr client = sock->accept();
            /// 创建新的连接后 用m_ioWorker将其调度到处理连接的线程上
            if(client) {
                client->setRecvTimeout(m_recvTimeout);
                /// 分配handleClient任务 handleClient用来写具体的业务逻辑
                m_ioWorker->schedule(std::bind(&TcpServer::handleClient,
                                               shared_from_this(), client));
            } else {
                EUTERPE_LOG_ERROR(g_logger) << "accept errno=" << errno
                                          << " errstr=" << strerror(errno);
            }
        }
    }

    std::string TcpServer::toString(const std::string& prefix) {
        std::stringstream ss;
        ss << prefix << "[type=" << m_type
           << " name=" << m_name
           << " worker=" << (m_worker ? m_worker->getName() : "")
           << " accept=" << (m_acceptWorker ? m_acceptWorker->getName() : "")
           << " recv_timeout=" << m_recvTimeout << "]" << std::endl;
        std::string pfx = prefix.empty() ? "    " : prefix;
        for(auto& i : m_socks) {
            ss << pfx << pfx << *i << std::endl;
        }
        return ss.str();
    }
} // euterpe