//
// Created by hongzhe on 22-8-3.
//

#include <iostream>
#include "../src/euterpe.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

euterpe::Logger::ptr fiber_g_logger = EUTERPE_LOG_ROOT();

void test_fiber(){
    EUTERPE_LOG_INFO(fiber_g_logger) << "test_fiber_function";
}

void test1(){
    euterpe::IOManager* iom = new euterpe::IOManager(1);
    // iom.schedule(&test_fiber);

    int sock = socket(AF_INET,SOCK_STREAM,0);
    fcntl(sock,F_SETFL,O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr.s_addr);

    iom->addEvent(sock,euterpe::IOManager::READ,[](){
        EUTERPE_LOG_INFO(fiber_g_logger) << "function be used";
    });
    delete iom;
    // int rt = connect(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
}

euterpe::Timer::ptr s_timer;

void test_time(){
    euterpe::IOManager iom(2);
    s_timer = iom.addTimer(1000,[](){
        static int i = 0;
        EUTERPE_LOG_INFO(fiber_g_logger) << "hello timer i=" << i;
        if(++i==5){
            s_timer->cancel();
        }
    },1);
}

int main(int argc, char** argv){
    test_time();
    return 0;
}