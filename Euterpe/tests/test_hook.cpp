//
// Created by hongzhe on 22-8-7.
//

#include "../src/euterpe.h"
#include <iostream>

euterpe::Logger::ptr fiber_g_logger = EUTERPE_LOG_ROOT();

void test_1(){
    euterpe::IOManager iom(1);
    iom.schedule([](){
        EUTERPE_LOG_INFO(fiber_g_logger) << "test1" ;
        sleep(5);
        EUTERPE_LOG_INFO(fiber_g_logger) << "test2" ;
    });
}

int main(){
    test_1();
    return 0;
}