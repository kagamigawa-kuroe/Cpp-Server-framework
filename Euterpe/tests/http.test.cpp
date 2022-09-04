////
//// Created by hongzhe on 22-8-31.
////
//
//#include "../src/euterpe.h"
//
//static euterpe::Logger::ptr g_logger = EUTERPE_LOG_ROOT();
//
//const char test_request_data[] = "POST / HTTP/1.1\r\n"
//                                 "Host: www.euterpe.top\r\n"
//                                 "Content-Length: 10\r\n\r\n"
//                                 "1234567890";
//
////void test_request() {
////    euterpe::http::HttpRequest::ptr req(new euterpe::http::HttpRequest);
////    req->setHeader("host" , "www.euterpe.top");
////    req->setBody("hello euterpe");
////    req->dump(std::cout) << std::endl;
////}
////
////void test_response() {
////    euterpe::http::HttpResponse::ptr rsp(new euterpe::http::HttpResponse);
////    rsp->setHeader("X-X", "whz");
////    rsp->setBody("hello whz");
////    rsp->setStatus((euterpe::http::HttpStatus)400);
////    rsp->setClose(false);
////
////    rsp->dump(std::cout) << std::endl;
////}
//
//void test_request() {
//    euterpe::http::HttpRequestParser parser;
//    std::string tmp = test_request_data;
//    size_t s = parser.execute(&tmp[0], tmp.size());
//    EUTERPE_LOG_ERROR(g_logger) << "execute rt=" << s
//                              << "has_error=" << parser.hasError()
//                              << " is_finished=" << parser.isFinished()
//                              << " total=" << tmp.size()
//                              << " content_length=" << parser.getContentLength();
//    tmp.resize(tmp.size() - s);
//    EUTERPE_LOG_INFO(g_logger) << parser.getData()->toString();
//    EUTERPE_LOG_INFO(g_logger) << tmp;
//}
//
//const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
//                                  "Date: Tue, 04 Jun 2019 15:43:56 GMT\r\n"
//                                  "Server: Apache\r\n"
//                                  "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
//                                  "ETag: \"51-47cf7e6ee8400\"\r\n"
//                                  "Accept-Ranges: bytes\r\n"
//                                  "Content-Length: 81\r\n"
//                                  "Cache-Control: max-age=86400\r\n"
//                                  "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
//                                  "Connection: Close\r\n"
//                                  "Content-Type: text/html\r\n\r\n"
//                                  "<html>\r\n"
//                                  "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
//                                  "</html>\r\n";
//
//void test_response() {
//    euterpe::http::HttpResponseParser parser;
//    std::string tmp = test_response_data;
//    size_t s = parser.execute(&tmp[0], tmp.size(), true);
//    EUTERPE_LOG_ERROR(g_logger) << "execute rt=" << s
//                              << " has_error=" << parser.hasError()
//                              << " is_finished=" << parser.isFinished()
//                              << " total=" << tmp.size()
//                              << " content_length=" << parser.getContentLength()
//                              << " tmp[s]=" << tmp[s];
//
//    tmp.resize(tmp.size() - s);
//
//    EUTERPE_LOG_INFO(g_logger) << parser.getData()->toString();
//    // EUTERPE_LOG_INFO(g_logger) << tmp;
//}
//
//
//int main(int argc, char** argv) {
//    // test_request();
//    test_response();
//    return 0;
//}