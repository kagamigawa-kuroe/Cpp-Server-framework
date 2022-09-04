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

    while(!server->bind(addr)) {
    }
    server->start();
}

int main(int argc, char** argv) {
    euterpe::IOManager iom(2);
    iom.schedule(run);
    return 0;
}