//
// Created by 王泓哲 on 12/07/2022.
//
#include "../src/euterpe.h"
#include <iostream>
#include "../src/thread/mutex.h"
#include <mutex>
using namespace std;

euterpe::Logger::ptr g_logger = EUTERPE_LOG_ROOT();

std::mutex mtx;
void fun2() {
    mtx.lock();
    EUTERPE_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    mtx.unlock();
}

int main(){
    EUTERPE_LOG_INFO(g_logger) << "thread test begin";
    YAML::Node root = YAML::LoadFile("/Users/whz/learning/Cpp-Server-framework/Euterpe/bin/conf/log.yml");
    euterpe::Config::LoadFromYaml(root);

    std::vector<euterpe::Thread::ptr> thrs;
    for(int i = 0; i < 5; ++i) {
        euterpe::Thread::ptr thr(new euterpe::Thread(&fun2, "name_" + std::to_string(i * 2)));
        thrs.push_back(thr);
    }

    for(size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
    EUTERPE_LOG_INFO(g_logger) << "thread test end";
    return 0;
}

