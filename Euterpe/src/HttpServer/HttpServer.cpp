//
// Created by hongzhe on 22-9-4.
//

#include "HttpServer.h"

namespace euterpe {
    namespace http {
        static euterpe::Logger::ptr g_logger = EUTERPE_LOG_NAME("system");

        HttpServer::HttpServer(bool keepalive
                ,IOManager* worker
                ,IOManager* io_worker
                ,IOManager* accept_worker)
                :TcpServer(worker, io_worker, accept_worker)
                ,m_isKeepalive(keepalive) {
            m_type = "http";
        }

        void HttpServer::setName(const std::string &mName) {
            TcpServer::setName(mName);
        }

        /// 重写tcpserver的处理连接函数
        void HttpServer::handleClient(Socket::ptr client) {
            HttpSession::ptr session(new HttpSession(client));
            do {
                /// 从session中 读取一个完整的http请求
                /// 这部分功能由HttpSession类实现
                /// 大致流程为: 从socket中读取特定长度的消息->用HttpParser解析->返回HttpResponse
                auto req = session->recvRequest();

                /// 如果读取失败 报错
                if(!req) {
                    EUTERPE_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                                              << errno << " errstr=" << strerror(errno)
                                              << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
                    break;
                }

                /// 创建返回的HttpResponse
                HttpResponse::ptr rsp(new HttpResponse(req->getVersion()
                        ,req->isClose() || !m_isKeepalive));

                /// 接下来 根据请求的内容 设置HttpResponse中对应的内容 通过Session返回
                rsp->setHeader("Server", getName());
                rsp->setBody("hello euterpe");
                session->sendResponse(rsp);

                /// 如果为短连接 或者 连接已经断开 则退出循环
                if(!m_isKeepalive || req->isClose()) {
                    break;
                }
            } while(true);

            /// 关闭session
            session->close();
        }

    } // euterpe
} // http