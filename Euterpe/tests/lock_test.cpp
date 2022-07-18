//
// Created by hongzhe on 22-7-15.
//

#include "../src/euterpe.h"
#include <iostream>

int count = 0;
euterpe::Mutex mtx;
euterpe::Mutex::Lock p(mtx);
euterpe::Logger::ptr g_logger2 = EUTERPE_LOG_ROOT();

void fun22() {
    EUTERPE_LOG_INFO(g_logger2) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
}

void fun1(){
    for (int i = 0; i < 10; ++i) {
        p.lock();
        ++count;
        p.unlock();
        sleep(1);
    }
}
//\
//int main(){
//    std::vector<euterpe::Thread::ptr> thrs;
//    for(int i = 0; i < 5; ++i) {
//        euterpe::Thread::ptr thr(new euterpe::Thread(&fun22, "name_" + std::to_string(i * 2)));
//        thrs.push_back(thr);
//    }
//
//    for(size_t i = 0; i < thrs.size(); ++i) {
//        thrs[i]->join();
//    }
//
//}