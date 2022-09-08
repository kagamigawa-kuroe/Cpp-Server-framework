//
// Created by hongzhe on 22-9-4.
//

#include "HttpSession.h"
#include <iostream>
#include <utility>
#include "http.h"
#include "httpclient_parser.h"
#include "http_parser.h"
#include "../Log/log.h"

namespace euterpe {
    namespace http {
        static euterpe::Logger::ptr g_logger = EUTERPE_LOG_ROOT();
        /// 构造函数 传入一个socket
        HttpSession::HttpSession(Socket::ptr sock, bool owner)
                :SocketStream(std::move(sock), owner) {
        }

        HttpRequest::ptr HttpSession::recvRequest() {
            /// 构造一个请求解析器
            HttpRequestParser::ptr parser(new HttpRequestParser);

            /// 获取请求默认的size
            uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();

            /// 获取一块内存的智能指针
            std::shared_ptr<char> buffer(
                    new char[buff_size], [](char* ptr){
                        delete[] ptr;
                    });

            /// 获取内存块
            char* data = buffer.get();
            int offset = 0;
            do {
                /// 从上次独到的地方开始 读取剩下需要的大小
                int len = read(data + offset, buff_size - offset);
                /// std::cout << data[0] << data[1] << data[2] << std::endl;

                /// 如果读取失败 就返回
                if(len <= 0) {
                    close();
                    return nullptr;
                }

                /// len + 原本的offset 就是现在已经读到的总长度
                len += offset;

                /// 对现在的总长度进行解析
                size_t nparse = parser->execute(data, len);

                /// 出错 则关闭
                if(parser->hasError()) {
                    close();
                    return nullptr;
                }

                /// 减去已经解析完成的长度 剩下的长度就是下次重新开始读取的偏移量
                /// 因为就算其已经被读取过了 但是没有被解析 所以需要重新读取
                offset = len - nparse;

                /// 如果超过了默认值 则退出
                if(offset == (int)buff_size) {
                    close();
                    return nullptr;
                }

                /// 解析已经结束了 就退出
                if(parser->isFinished()) {
                    break;
                }

            } while(true);

            /// 获取请求体长度
            int64_t length = parser->getContentLength();

            // std::cout << length << " " << offset << std::endl;
            // std::cout << *data  << std::endl;
            if(length > 0) {
                std::string body;
                body.resize(length);

                int len = 0;

                /// 读length个 除非offset小于length 就读offset个
                if(length >= offset) {
                    memcpy(&body[0], data, offset);
                    len = offset;
                } else {
                    memcpy(&body[0], data, length);
                    len = length;
                }
                length -= offset;

                //// 只有length大于偏移量的时候 才读取剩下的内容
                if(length > 0) {
                    if(readFixSize(&body[len], length) <= 0) {
                        close();
                        return nullptr;
                    }
                }
                parser->getData()->setBody(body);
                // EUTERPE_LOG_ERROR(g_logger) << body << " " << body.size();
            }

            parser->getData()->init();
            return parser->getData();
        }

        /// rsp无需特殊的长度处理
        /// 直接返回即可
        int HttpSession::sendResponse(HttpResponse::ptr rsp) {
            std::stringstream ss;
            ss << *rsp;
            std::string data = ss.str();
            return writeFixSize(data.c_str(), data.size());
        }
    } // euterpe
} // http