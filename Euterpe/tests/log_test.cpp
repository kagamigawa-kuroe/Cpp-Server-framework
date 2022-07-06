#include <iostream>
#include <vector>
#include "../src/utils/utils.h"
#include "../src/Log/log.h"
#include "../src/config/config.h"

int test_base_cout_put(){
    euterpe::Logger::ptr logger(new euterpe::Logger);

    /// logger在构造的时候就会有一个默认的格式
    logger->addAppender(euterpe::LogAppender::ptr(new euterpe::StdoutLogAppender));
    euterpe::LogEvent::ptr event(new euterpe::LogEvent(logger,euterpe::LogLevel::Level::DEBUG,
                                                       __FILE__,__LINE__,0,euterpe::GetThreadId(),
                                                       euterpe::GetFiberId(), time(0),"test"));
    event->getSS() << "event 1";
    logger->log(euterpe::LogLevel::DEBUG,event);
    return 0;
}

void test_macro_log_wrapper(){
    euterpe::Logger::ptr logger(new euterpe::Logger);
    logger->addAppender(euterpe::LogAppender::ptr(new euterpe::StdoutLogAppender));
    EUTERPE_LOG_ERROR(logger)<<"1234";
}

void test_fmt_out_file(){
    euterpe::Logger::ptr logger(new euterpe::Logger);
    euterpe::FileLogAppender::ptr file_appender(new euterpe::FileLogAppender("/Users/whz/learning/Cpp-Server-framework/Euterpe/tests/log.txt"));
    logger->addAppender(file_appender);
    EUTERPE_LOG_DEBUG(logger)<<"1234";
    EUTERPE_LOG_FMT_ERROR(logger,"test %s","helo");
};

void test_manager(){
    auto l = euterpe::LoggerMgr::GetInstance()->getLogger("xx");
    EUTERPE_LOG_DEBUG(l) << "loggernot find";
    auto l2 = EUTERPE_LOG_ROOT();
    EUTERPE_LOG_DEBUG(l2)<<"1234";
}

//int main() {
//   test_manager();
//
//}
