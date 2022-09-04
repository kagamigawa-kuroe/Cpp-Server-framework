//
// Created by hongzhe on 22-9-4.
//

#ifndef EUTERPE_HTTPSERVER_H
#define EUTERPE_HTTPSERVER_H
#include "../tcp_server/tcp_server.h"
#include "../http/HttpSession.h"
#include <memory>
namespace euterpe {
    namespace http{
        class HttpServer: public TcpServer{
        public:
            typedef std::shared_ptr<HttpServer> ptr;
            explicit HttpServer(bool keepalive = false
                    ,IOManager* worker = IOManager::GetThis()
                    ,IOManager* io_worker = IOManager::GetThis()
                    ,IOManager* accept_worker = IOManager::GetThis());

            void setName(const std::string &mName) override;

        protected:
            void handleClient(Socket::ptr client) override;

        private:
            /// 是否支持长连接
            bool m_isKeepalive;
        };

    } // euterpe
} // http

#endif //EUTERPE_HTTPSERVER_H
