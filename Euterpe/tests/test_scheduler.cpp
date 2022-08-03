//
// Created by hongzhe on 22-7-24.
//
//#include "../src/euterpe.h"
// euterpe::Logger::ptr fiber_g_logger = EUTERPE_LOG_ROOT();
//
//void test_fiber(){
//    EUTERPE_LOG_INFO(fiber_g_logger) << "------------------------------------";
//    EUTERPE_LOG_INFO(fiber_g_logger) << "test";
//    // euterpe::Scheduler::GetThis()->schedule(&test_fiber);
//}
//
//int main(){
//    euterpe::Scheduler sc = euterpe::Scheduler(1);
//    sc.start();
//    sc.schedule(&test_fiber);
//    sc.stop();
//    return 0;
//}