//
// Created by hongzhe on 22-8-31.
//

#ifndef EUTERPE_HTTP_PARSER_H
#define EUTERPE_HTTP_PARSER_H

#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"

namespace euterpe {
    namespace http {

/// 请求解析类
/// 封装一个HttpRequest 和一个 Http11_parser

        class HttpRequestParser {
        public:

            /// HTTP解析类的智能指针
            typedef std::shared_ptr<HttpRequestParser> ptr;

            HttpRequestParser();

            /// 解析函数 数据 长度
            size_t execute(char* data, size_t len);

            /// 是否解析完成
            int isFinished();

            /// 是否发生错误
            int hasError();

            /// 返回其中的HttpRequest结构体
            HttpRequest::ptr getData() const { return m_data;}

            /// 设置发生错误
            void setError(int v) { m_error = v;}

            /// 获取消息体长度
            uint64_t getContentLength();

            /// 获取解析结构体
            const http_parser& getParser() const { return m_parser;}
        public:
            /**
             * @brief 返回HttpRequest协议解析的缓存大小
             */
            static uint64_t GetHttpRequestBufferSize();

            /**
             * @brief 返回HttpRequest协议的最大消息体大小
             */
            static uint64_t GetHttpRequestMaxBodySize();
        private:
            /// http_parser
            http_parser m_parser;
            /// HttpRequest结构
            HttpRequest::ptr m_data;
            /// 错误码
            /// 1000: invalid method
            /// 1001: invalid version
            /// 1002: invalid field
            int m_error;
        };

/// 响应解析结构体
        class HttpResponseParser {
        public:
            /// 智能指针类型
            typedef std::shared_ptr<HttpResponseParser> ptr;

            HttpResponseParser();

            size_t execute(char* data, size_t len, bool chunck);

            int isFinished();

            int hasError();

            HttpResponse::ptr getData() const { return m_data;}

            void setError(int v) { m_error = v;}

            uint64_t getContentLength();

            const httpclient_parser& getParser() const { return m_parser;}
        public:
            /**
             * @brief 返回HTTP响应解析缓存大小
             */
            static uint64_t GetHttpResponseBufferSize();

            /**
             * @brief 返回HTTP响应最大消息体大小
             */
            static uint64_t GetHttpResponseMaxBodySize();
        private:
            /// httpclient_parser
            httpclient_parser m_parser;
            /// HttpResponse
            HttpResponse::ptr m_data;
            /// 错误码
            /// 1001: invalid version
            /// 1002: invalid field
            int m_error;
        };

    }
}

#endif //EUTERPE_HTTP_PARSER_H
