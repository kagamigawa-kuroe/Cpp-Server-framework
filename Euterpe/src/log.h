#ifndef _EUTERPE_LOG_H_
#define _EUTERPE_LOG_H_

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>


namespace euterpe{
    class LogAppender;
    //日志级别
    class LogLevel{
    public:
        enum Level{
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };
    };

    // log触发事件
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr; 
        LogEvent() = default;
    private:
        const char* m_file = nullptr; //文件名
        int32_t m_line = 0; //行号
        int32_t m_threadId = 0; //线程id
        uint32_t m_elapse = 0; //程序启动经过的时间
        uint32_t m_fiberId = 0; //携程id
        uint64_t m_time; //时间
        std::string m_content; //内容
    };

    //日志格式
    class LogFormatter{
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        std::string format(LogEvent::ptr event);

    private:

    };

    //日志输出地
    class LogAppender{
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        LogAppender() = default;
        virtual ~LogAppender();
        virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;
        LogFormatter::ptr getFormatter() const {return m_formatter;};
        LogLevel::Level getlevel() const {return m_level;};
    private:
        LogLevel::Level m_level;
        LogFormatter::ptr m_formatter;
    };

    //日志生成器
    class Logger{
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        Logger() = default;
        Logger(const std::string& name = "root");
        void log(LogLevel::Level level, LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void addApppender(LogAppender::ptr appender);
        void delApppender(LogAppender::ptr appender);

        LogLevel::Level getLevel() const {return m_level;};
        void setLevel(LogLevel::Level val) {m_level = val;};
    private:
        std::string m_name;                      //日志名称
        LogLevel::Level m_level;                 //只有满足这个日志级别的日志才会被输出
        std::list<LogAppender::ptr> m_appenders; //输出目的地的集合
    };

    //输出到控制台
    class StdoutLogAppender:public LogAppender{
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        virtual void log(LogLevel::Level level, LogEvent::ptr event) override;

    };

    //输出到文件
    class FileLogAppender:public LogAppender{
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string& filename);
        virtual void log(LogLevel::Level level, LogEvent::ptr event) override;
        bool reopen();
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };
}

#endif