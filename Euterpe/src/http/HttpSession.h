//
// Created by hongzhe on 22-9-4.
//

#ifndef EUTERPE_HTTPSESSION_H
#define EUTERPE_HTTPSESSION_H
#include <memory>
#include "../Network_Base/Socket.h"
#include "../stream/SocketStream.h"
#include "http.h"

namespace euterpe {
    namespace http {
        /**
         * @brief 用于协调socket 以及HttpRequest和Response
         * @brief 通过这各类从Socket描述符中读取信息解析为HttpRequest类
         * @brief 以及通过HttpResponse类向socket描述符中发送http协议请求
         */

        /**
         * @brief 本质就是封装了一个socket描述符 也就是一个tcp连接 负责从其中读出一个个httprequest 并写入response
         */
        class HttpSession: public SocketStream {
        public:
            typedef std::shared_ptr<HttpSession> ptr;
        public:
            HttpSession() = default;
            HttpSession(Socket::ptr sock, bool owner = true);
            /// 发送和接受http请求
            HttpRequest::ptr recvRequest();
            int sendResponse(HttpResponse::ptr rsp);
        };

    } // euterpe
} // http

#endif //EUTERPE_HTTPSESSION_H
