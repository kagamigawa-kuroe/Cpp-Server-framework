//
// Created by hongzhe on 22-8-31.
//

#ifndef EUTERPE_HTTP_H
#define EUTERPE_HTTP_H

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>

namespace euterpe {
    namespace http {

/* Request Methods */
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       \

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \


/// HTTP方法枚举类 枚举类和普通枚举的区别在于作用域
/// 枚举类在使用枚举成员的时候 需要加上类名作为作用域

        enum class HttpMethod {
#define XX(num, name, string) name = num,
            HTTP_METHOD_MAP(XX)
#undef XX
            INVALID_METHOD
        };

/// HTTP状态枚举
        enum class HttpStatus {
#define XX(code, name, desc) name = code,
            HTTP_STATUS_MAP(XX)
#undef XX
        };

/// 将字符串方法名转成HTTP方法枚举
        HttpMethod StringToHttpMethod(const std::string& m);

/// 将字符串指针转换成HTTP方法枚举
        HttpMethod CharsToHttpMethod(const char* m);

/// 将方法转换成字符串指针
        const char* HttpMethodToString(const HttpMethod& m);

/// 将状态转换成字符串指针
        const char* HttpStatusToString(const HttpStatus& s);

/// 忽略大小写的字符串比较函数
        struct CaseInsensitiveLess {
            bool operator()(const std::string& lhs, const std::string& rhs) const;
        };

/// 模板 获取Map中的key值 存储在val引用中 并转成对应类型 返回是否成功
        template<class MapType, class T>
        bool checkGetAs(const MapType& m, const std::string& key, T& val, const T& def = T()) {
            auto it = m.find(key);
            if(it == m.end()) {
                val = def;
                return false;
            }
            try {
                val = boost::lexical_cast<T>(it->second);
                return true;
            } catch (...) {
                val = def;
            }
            return false;
        }

/// 模板 功能同上 只不过返回值为获取到的变量 而不使用引用返回
        template<class MapType, class T>
        T getAs(const MapType& m, const std::string& key, const T& def = T()) {
            auto it = m.find(key);
            if(it == m.end()) {
                return def;
            }
            try {
                return boost::lexical_cast<T>(it->second);
            } catch (...) {
            }
            return def;
        }

        class HttpResponse;

/// http请求结构 类似于java HttpRequest
        class HttpRequest {
        public:
            /// HTTP请求的智能指针
            typedef std::shared_ptr<HttpRequest> ptr;

            /// MAP结构 在存储时 我们统一使用string进行存储
            typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;

            /// 构造函数 http版本 和 是否 keep-alive
            HttpRequest(uint8_t version = 0x11, bool close = true);

            /// 创造一个响应 返回智能指针
            std::shared_ptr<HttpResponse> createResponse();

            /// 获取请求的方法 get/post/delete...
            HttpMethod getMethod() const { return m_method;}

            /// 获取请求的版本
            uint8_t getVersion() const { return m_version;}

            /// 获取请求的url路经
            const std::string& getPath() const { return m_path;}

            /// 获取请求中的Query参数 用string返回
            const std::string& getQuery() const { return m_query;}

            /// 获取请求体正文
            const std::string& getBody() const { return m_body;}

            /// 获取请求头 用string - string map的形式存储
            const MapType& getHeaders() const { return m_headers;}

            /// 获取请求中的Query参数 用map返回
            const MapType& getParams() const { return m_params;}

            /// 用map返回所有的cookie
            const MapType& getCookies() const { return m_cookies;}

            /// 设置请求方法
            void setMethod(HttpMethod v) { m_method = v;}

            /// 设置版本
            void setVersion(uint8_t v) { m_version = v;}

            /// 设置请求路径
            void setPath(const std::string& v) { m_path = v;}

            /// 设置参数
            void setQuery(const std::string& v) { m_query = v;}

            /// 设置请求中的锚点(#)
            void setFragment(const std::string& v) { m_fragment = v;}

            /// 设置消息体
            void setBody(const std::string& v) { m_body = v;}

            /// 设置是已经关闭
            bool isClose() const { return m_close;}

            /// 设置是否为长链接
            void setClose(bool v) { m_close = v;}

            /// 是否是一个websocket协议
            bool isWebsocket() const { return m_websocket;}

            /// 设置是否是websocket协议
            void setWebsocket(bool v) { m_websocket = v;}

            /// 设置请求头的map
            void setHeaders(const MapType& v) { m_headers = v;}

            /// 设置请求参数的map
            void setParams(const MapType& v) { m_params = v;}

            /// 设置cookie的map
            void setCookies(const MapType& v) { m_cookies = v;}

            /// 通过key获取头部参数
            std::string getHeader(const std::string& key, const std::string& def = "") const;

            /// 通过key获取请求参数
            std::string getParam(const std::string& key, const std::string& def = "");

            /// 通过key获取cookie参数
            std::string getCookie(const std::string& key, const std::string& def = "");

            /// 设置一个头部的key-val
            void setHeader(const std::string& key, const std::string& val);

            /// 设置一个请求参数的KEY-VALUE
            void setParam(const std::string& key, const std::string& val);

            /// 设置一个COOKIE的 KEY-VALUE
            void setCookie(const std::string& key, const std::string& val);

            /// 删除头部信息
            void delHeader(const std::string& key);

            /// 删除请求参数信息
            void delParam(const std::string& key);

            /// 删除cookie信息
            void delCookie(const std::string& key);

            /// 判断头部参数是否存在
            bool hasHeader(const std::string& key, std::string* val = nullptr);

            /// 判断请求参数是否存在
            bool hasParam(const std::string& key, std::string* val = nullptr);

            /// 判断cookie是否存在
            bool hasCookie(const std::string& key, std::string* val = nullptr);

            /// 获取一个T类型的头部信息 注意之前都是用string类型直接返回的
            /// 以引用形式返回 返回值为bool
            template<class T>
            bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
                return checkGetAs(m_headers, key, val, def);
            }

            /// 直接以返回值的形式返回
            template<class T>
            T getHeaderAs(const std::string& key, const T& def = T()) {
                return getAs(m_headers, key, def);
            }

            /// 同上 只不过是获取请求参数
            template<class T>
            bool checkGetParamAs(const std::string& key, T& val, const T& def = T()) {
                initQueryParam();
                initBodyParam();
                return checkGetAs(m_params, key, val, def);
            }

            template<class T>
            T getParamAs(const std::string& key, const T& def = T()) {
                initQueryParam();
                initBodyParam();
                return getAs(m_params, key, def);
            }

            /// 同理 只不过是获取cookie参数
            template<class T>
            bool checkGetCookieAs(const std::string& key, T& val, const T& def = T()) {
                initCookies();
                return checkGetAs(m_cookies, key, val, def);
            }

            template<class T>
            T getCookieAs(const std::string& key, const T& def = T()) {
                initCookies();
                return getAs(m_cookies, key, def);
            }

            /// 将信息格式化出入到流中
            std::ostream& dump(std::ostream& os) const;

            /// 以字符串形式输出
            std::string toString() const;

            void init();
            void initParam();
            void initQueryParam();
            void initBodyParam();
            void initCookies();

        private:
            /// HTTP方法
            HttpMethod m_method;
            /// HTTP版本
            uint8_t m_version;
            /// 是否自动关闭
            bool m_close;
            /// 是否为websocket
            bool m_websocket;

            uint8_t m_parserParamFlag;
            /// 请求路径
            std::string m_path;
            /// 请求参数
            std::string m_query;
            /// 请求fragment
            std::string m_fragment;
            /// 请求消息体
            std::string m_body;
            /// 请求头部MAP
            MapType m_headers;
            /// 请求参数MAP
            MapType m_params;
            /// 请求Cookie MAP
            MapType m_cookies;
        };

/// HTTP响应结构体
        class HttpResponse {
        public:
            /// HTTP响应结构智能指针
            typedef std::shared_ptr<HttpResponse> ptr;
            /// MapType
            typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;

            HttpResponse(uint8_t version = 0x11, bool close = true);

            HttpStatus getStatus() const { return m_status;}

            uint8_t getVersion() const { return m_version;}


            const std::string& getBody() const { return m_body;}

            const std::string& getReason() const { return m_reason;}

            const MapType& getHeaders() const { return m_headers;}

            void setStatus(HttpStatus v) { m_status = v;}

            void setVersion(uint8_t v) { m_version = v;}

            void setBody(const std::string& v) { m_body = v;}

            void setReason(const std::string& v) { m_reason = v;}

            void setHeaders(const MapType& v) { m_headers = v;}

            bool isClose() const { return m_close;}

            void setClose(bool v) { m_close = v;}

            bool isWebsocket() const { return m_websocket;}

            void setWebsocket(bool v) { m_websocket = v;}

            std::string getHeader(const std::string& key, const std::string& def = "") const;

            void setHeader(const std::string& key, const std::string& val);

            void delHeader(const std::string& key);

            template<class T>
            bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
                return checkGetAs(m_headers, key, val, def);
            }

            template<class T>
            T getHeaderAs(const std::string& key, const T& def = T()) {
                return getAs(m_headers, key, def);
            }

            std::ostream& dump(std::ostream& os) const;

            std::string toString() const;

            void setRedirect(const std::string& uri);

            void setCookie(const std::string& key, const std::string& val,
                           time_t expired = 0, const std::string& path = "",
                           const std::string& domain = "", bool secure = false);
        private:
            /// 响应状态
            HttpStatus m_status;
            /// 版本
            uint8_t m_version;
            /// 是否自动关闭
            bool m_close;
            /// 是否为websocket
            bool m_websocket;
            /// 响应消息体
            std::string m_body;
            /// 响应原因
            std::string m_reason;
            /// 响应头部MAP
            MapType m_headers;

            std::vector<std::string> m_cookies;
        };

        std::ostream& operator<<(std::ostream& os, const HttpRequest& req);
        std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp);

    }
}


#endif //EUTERPE_HTTP_H
