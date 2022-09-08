//
// Created by hongzhe on 22-9-4.
//

#ifndef EUTERPE_HTTPSERVER_H
#define EUTERPE_HTTPSERVER_H
#include "../tcp_server/tcp_server.h"
#include "../http/HttpSession.h"
#include <memory>
#include "../servlet/Servlet.h"

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

            /**
             * @brief 获取ServletDispatch
             */
            ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}

            /**
             * @brief 设置ServletDispatch
             */
            void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}

        protected:
            /// tcp绑定端口 tcp调用start tcp调用startconnect startconnect拿到一个socket -----
            /// 调用分配一个handleClient任务 任务内通过上面的socket建立一个session 通过session读取请求 处理业务 和响应
            void handleClient(Socket::ptr client) override;

        private:
            /// 是否支持长连接
            bool m_isKeepalive;
            ServletDispatch::ptr m_dispatch;
        };

    } // euterpe
} // http

#endif //EUTERPE_HTTPSERVER_H
