//
// Created by hongzhe on 22-7-24.
//
#include "../src/euterpe.h"
euterpe::Logger::ptr fiber_g_logger = EUTERPE_LOG_ROOT();

//int main(){
//    euterpe::Scheduler sc = euterpe::Scheduler();
//    sc.start();
//    sc.stop();
//    return 0;
//}
void test_fiber(){
    EUTERPE_LOG_INFO(fiber_g_logger) << "test1";
}

int main(){
    euterpe::Scheduler sc = euterpe::Scheduler(2);
    sc.start();
    sc.schedule(&test_fiber);
    sc.stop();
    return 0;
}