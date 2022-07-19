//
// Created by hongzhe on 22-7-19.
//
#include <iostream>
#include "../src/euterpe.h"

euterpe::Logger::ptr fiber_g_logger = EUTERPE_LOG_ROOT();

void run_in_fiber(){
    EUTERPE_LOG_INFO(fiber_g_logger) << "fiber 1 begin";
    euterpe::Fiber::YieldToHold();
    EUTERPE_LOG_INFO(fiber_g_logger) << "fiber 2 begin";
    // euterpe::Fiber::YieldToHold();
}

int main(){
    euterpe::Fiber::GetThis();
    EUTERPE_LOG_INFO(fiber_g_logger) << "test begin";
    EUTERPE_LOG_INFO(fiber_g_logger) << "-------------------------------------";
    euterpe::Fiber::ptr fiber(new euterpe::Fiber(run_in_fiber));
    fiber->swapIn();
    EUTERPE_LOG_INFO(fiber_g_logger) << "stop";
    fiber->swapIn();
    EUTERPE_LOG_INFO(fiber_g_logger) << "-------------------------------------";
    EUTERPE_LOG_INFO(fiber_g_logger) << "test end";
}