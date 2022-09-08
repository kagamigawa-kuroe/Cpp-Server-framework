//
// Created by hongzhe on 22-9-4.
//

#include "../src/euterpe.h"

static euterpe::Logger::ptr g_logger = EUTERPE_LOG_ROOT();

#define XX(...) #__VA_ARGS__


euterpe::IOManager::ptr worker;
void run() {
    g_logger->setLevel(euterpe::LogLevel::INFO);
    euterpe::http::HttpServer::ptr server(new euterpe::http::HttpServer(true));
    euterpe::Address::ptr addr = euterpe::Address::LookupAnyIPAddress("0.0.0.0:8020");
    server->bind(addr);
    auto sd = server->getServletDispatch();
    sd->addServlet("/euterpe/xx", [](euterpe::http::HttpRequest::ptr req
            ,euterpe::http::HttpResponse::ptr rsp
            ,euterpe::http::HttpSession::ptr session) {
        rsp->setBody(req->toString());
        return 0;
    });

    sd->addGlobServlet("/euterpe/*", [](euterpe::http::HttpRequest::ptr req
            ,euterpe::http::HttpResponse::ptr rsp
            ,euterpe::http::HttpSession::ptr session) {
        rsp->setBody("Glob:\r\n" + req->toString());
        return 0;
    });

    sd->addGlobServlet("/euterpex/*", [](euterpe::http::HttpRequest::ptr req
            ,euterpe::http::HttpResponse::ptr rsp
            ,euterpe::http::HttpSession::ptr session) {
        rsp->setBody(XX(<html>
                                <head><title>test my servlet</title></head>
                                <body>
                                <center><h1>hello world</h1></center>
                                <hr><center>euterpe/1.0.0</center>
                                </body>
                                </html>
                     ));
        return 0;
    });
    server->start();
}

int main(int argc, char** argv) {
    euterpe::IOManager iom(2);
    iom.schedule(run);
    return 0;
}