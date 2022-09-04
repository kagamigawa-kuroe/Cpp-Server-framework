////
//// Created by hongzhe on 22-9-3.
////
//
//#include "../src/euterpe.h"
//
//static euterpe::Logger::ptr g_logger = EUTERPE_LOG_ROOT();
//
//class EchoServer : public euterpe::TcpServer {
//public:
//    EchoServer(int type);
//    void handleClient(euterpe::Socket::ptr client);
//
//private:
//    int m_type = 0;
//};
//
//EchoServer::EchoServer(int type)
//        :m_type(type) {
//}
//
//void EchoServer::handleClient(euterpe::Socket::ptr client) {
//    EUTERPE_LOG_INFO(g_logger) << "handleClient " << *client;
//    euterpe::ByteArray::ptr ba(new euterpe::ByteArray);
//    while(true) {
//        ba->clear();
//        std::vector<iovec> iovs;
//        ba->getWriteBuffers(iovs, 1024);
//
//        int rt = client->recv(&iovs[0], iovs.size());
//        if(rt == 0) {
//            EUTERPE_LOG_INFO(g_logger) << "client close: " << *client;
//            break;
//        } else if(rt < 0) {
//            EUTERPE_LOG_INFO(g_logger) << "client error rt=" << rt
//                                     << " errno=" << errno << " errstr=" << strerror(errno);
//            break;
//        }
//        ba->setPosition(ba->getPosition() + rt);
//        ba->setPosition(0);
//        if(m_type == 1) {//text
//            std::cout << ba->toString();// << std::endl;
//        } else {
//            std::cout << ba->toHexString();// << std::endl;
//        }
//        std::cout.flush();
//    }
//}
//
//int type = 1;
//
//void run() {
//    EUTERPE_LOG_INFO(g_logger) << "server type=" << type;
//    EchoServer::ptr es(new EchoServer(type));
//    auto addr = euterpe::Address::LookupAny("0.0.0.0:8020");
//    while(!es->bind(addr)) {
//        sleep(2);
//    }
//    es->start();
//}
//
//int main(int argc, char** argv) {
//    euterpe::IOManager iom(2);
//    iom.schedule(run);
//    return 0;
//}