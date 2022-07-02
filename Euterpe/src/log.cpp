#include "log.h"
#include <iostream>
namespace euterpe{
    Logger::Logger(const std::string& name){

    };

    void Logger::addApppender(LogAppender::ptr appender){
        m_appenders.push_back(appender);
    };

    void Logger::delApppender(LogAppender::ptr appender){
        for(auto it = m_appenders.begin();it!=m_appenders.end();++it){
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            } 
        }
    };

    void Logger::log(LogLevel::Level level, LogEvent::ptr event){
        if (level >= m_level)
        {
            for (auto& i : m_appenders)
            {
                i->log(level,event);
            }
        }
    };
    void Logger::debug(LogEvent::ptr event){
        log(LogLevel::DEBUG,event);
    };
    void Logger::info(LogEvent::ptr event){
        log(LogLevel::INFO,event);
    };
    void Logger::warn(LogEvent::ptr event){
        log(LogLevel::WARN,event);
    };
    void Logger::fatal(LogEvent::ptr event){
        log(LogLevel::FATAL,event);
    };
    void Logger::error(LogEvent::ptr event){
        log(LogLevel::ERROR,event);
    };

    void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event){
        if(level >= this->getlevel()){
            std::cout<<(this->getFormatter())->format(event);
        }
    };

    void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event){
        if(level>=this->getlevel()){
            m_filestream << (this->getFormatter())->format(event);
        }
    };

    FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename) {
        reopen();
    }

    bool FileLogAppender::reopen(){
        if(m_filestream){
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        return !!m_filestream;
    }
}