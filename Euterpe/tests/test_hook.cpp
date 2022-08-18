//
// Created by hongzhe on 22-8-7.
//

#include "../src/euterpe.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

euterpe::Logger::ptr fiber_g_logger = EUTERPE_LOG_ROOT();

//void test_1(){
//    euterpe::IOManager iom(1);
//    iom.schedule([](){
//        EUTERPE_LOG_INFO(fiber_g_logger) << "test1" ;
//        sleep(5);
//        EUTERPE_LOG_INFO(fiber_g_logger) << "test2" ;
//    });
//}

void test_2(){
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "172.217.22.142", &addr.sin_addr.s_addr);

    EUTERPE_LOG_INFO(fiber_g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    EUTERPE_LOG_INFO(fiber_g_logger) << "connect rt=" << rt << " errno=" << errno;

    if(rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    EUTERPE_LOG_INFO(fiber_g_logger) << "send rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    EUTERPE_LOG_INFO(fiber_g_logger) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    buff.resize(rt);
    EUTERPE_LOG_INFO(fiber_g_logger) << buff;
}

int main(){
    euterpe::IOManager iom;
    iom.schedule(test_2);
    return 0;
}