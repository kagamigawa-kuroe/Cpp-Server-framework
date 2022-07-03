#include <iostream>
#include "../src/log.h"

int main() {
    euterpe::Logger::ptr logger(new euterpe::Logger);
    logger->addAppender(euterpe::LogAppender::ptr(new euterpe::StdoutLogAppender));

//    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
//             ,const char* file, int32_t line, uint32_t elapse
//             ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
//             ,const std::string& thread_name);

    euterpe::LogEvent::ptr event(new euterpe::LogEvent(logger,euterpe::LogLevel::Level::DEBUG,
                                                   __FILE__,__LINE__,0,0,0,
                                                   time(0),"test"));

    logger->log(euterpe::LogLevel::DEBUG,event);

    return 0;
}
