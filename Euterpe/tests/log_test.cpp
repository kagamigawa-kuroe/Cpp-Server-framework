#include <iostream>
#include <vector>
#include "../src/utils/utils.h"
#include "../src/log.h"

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

int main() {
    test_macro_log_wrapper();
}
