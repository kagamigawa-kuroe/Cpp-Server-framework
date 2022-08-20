//
// Created by hongzhe on 22-8-20.
//
#include "../src/euterpe.h"

static euterpe::Logger::ptr g_looger = EUTERPE_LOG_ROOT();

void test_socket() {
    euterpe::IPAddress::ptr addr = euterpe::Address::LookupAnyIPAddress("www.google.com");
    euterpe::Socket::ptr sock = euterpe::Socket::CreateTCP(addr);
    addr->setPort(80);

    EUTERPE_LOG_INFO(g_looger) << "addr=" << addr->toString();
    sock->connect(addr);
    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    buffs.resize(rt);
    EUTERPE_LOG_INFO(g_looger) << buffs;
}


int main(int argc, char** argv) {
    euterpe::IOManager iom(4);
    //iom.schedule(&test_socket);
    iom.schedule(&test_socket);
    return 0;
}