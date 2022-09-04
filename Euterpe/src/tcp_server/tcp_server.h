//
// Created by hongzhe on 22-9-2.
//

#ifndef EUTERPE_TCP_SERVER_H
#define EUTERPE_TCP_SERVER_H
#include <memory>
#include <functional>
#include <vector>
#include "../Log/log.h"
#include "../IO/IoManager.h"
#include "../Network_Base/Socket.h"
#include "../Network_Base/address.h"

namespace euterpe {

    class TcpServer :public std::enable_shared_from_this<TcpServer> {
    public:
        typedef std::shared_ptr<TcpServer> ptr;
        TcpServer(IOManager* worker = IOManager::GetThis(),
                IOManager* io_worker = IOManager::GetThis(),
                IOManager* accept_worker = IOManager::GetThis());
        virtual ~TcpServer();

        virtual bool bind(Address::ptr addr);
        virtual bool bind(std::vector<Address::ptr> addrs, std::vector<Address::ptr>& fails);

        virtual bool start();
        virtual bool stop();

        uint64_t getReadTimeout() const { return m_recvTimeout; }
        void setReadTimeout(const uint64_t t) { m_recvTimeout = t; }

        std::string getName() const { return m_name; }

        virtual void setName(const std::string &mName) { m_name = mName; }

        bool getIsStop() const { return m_isStop; }
        void setIsStop(bool mIsStop) { m_isStop = mIsStop; }
        virtual std::string toString(const std::string& prefix = "");

    protected:
        /**
         * @brief 处理新连接的Socket类
         */
        virtual void handleClient(Socket::ptr client);

        /**
         * @brief 开始接受连接
         */
        virtual void startAccept(Socket::ptr sock);

    private:
        /// 所有listen的socket 可能有多个 因为可能会监听多个地址
        std::vector<Socket::ptr> m_socks;

        /// 线程池
        IOManager* m_worker;
        IOManager* m_ioWorker;

        /// 服务器Socket接收连接的调度器
        IOManager* m_acceptWorker;

        /// 读超时时间 很久没有发消息的连接就视为已经断开
        uint64_t  m_recvTimeout;

        /// name
        std::string m_name;

        /// server是否在工作
        bool m_isStop;

    public:
        /// type
        std::string m_type = "tcp";
    };

} // euterpe

#endif //EUTERPE_TCP_SERVER_H
