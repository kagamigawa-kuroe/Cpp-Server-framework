//
// Created by hongzhe on 22-8-19.
//

#include "../src/euterpe.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

euterpe::Logger::ptr fiber_g_logger = EUTERPE_LOG_ROOT();

void test11(){
    std::vector<euterpe::Address::ptr> addrs;

    bool v = euterpe::Address::Lookup(addrs,"www.baidu.com");

    for(auto &t:addrs){
        EUTERPE_LOG_INFO(fiber_g_logger) << t->toString();
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<euterpe::Address::ptr, uint32_t> > results;

    bool v = euterpe::Address::GetInterfaceAddresses(results);
    if(!v) {
        EUTERPE_LOG_ERROR(fiber_g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i: results) {
        EUTERPE_LOG_INFO(fiber_g_logger) << i.first << " - " << i.second.first->toString() << " - "
                                 << i.second.second;
    }
}

void test_ipv4() {
    auto addr = euterpe::IPAddress::Create("www.baidu.com");
    // auto addr = euterpe::IPAddress::Create("127.0.0.8");
    if(addr) {
        EUTERPE_LOG_INFO(fiber_g_logger) << addr->toString();
    }
}

int main(){
//    euterpe::IPv4Address a = euterpe::IPv4Address(288,8080);
//    auto t = euterpe::IPv4Address::Create("1.0.0.127",8080);
//    std::cout << *(t.get()) << std::endl;
//    test11();
//    test_ipv4();
    test_iface();
    return 0;
}