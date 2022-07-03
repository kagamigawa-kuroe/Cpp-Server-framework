//
// Created by 王泓哲 on 03/07/2022.
//

#ifndef _EUTERPE_LOG_H_
#define _EUTERPE_LOG_H_

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdarg.h>
#include <map>
#include "util.h"

namespace euterpe {

    class Logger;
    class LoggerManager;

    class LogLevel {
    public:

        enum Level {
            /// 未知级别
            UNKNOW = 0,
            /// DEBUG 级别
            DEBUG = 1,
            /// INFO 级别
            INFO = 2,
            /// WARN 级别
            WARN = 3,
            /// ERROR 级别
            ERROR = 4,
            /// FATAL 级别
            FATAL = 5
        };

        static const char* ToString(LogLevel::Level level);
        static LogLevel::Level FromString(const std::string& str);
    };

    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;

        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
                 ,const char* file, int32_t line, uint32_t elapse
                 ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
                 ,const std::string& thread_name);


        //返回文件名
        const char* getFile() const { return m_file;}
        //返回行号
        int32_t getLine() const { return m_line;}
        //耗时
        uint32_t getElapse() const { return m_elapse;}
        //线程id
        uint32_t getThreadId() const { return m_threadId;}
        //协程id
        uint32_t getFiberId() const { return m_fiberId;}
        //时间
        uint64_t getTime() const { return m_time;}
        //线程名
        const std::string& getThreadName() const { return m_threadName;}
        //日志内容
        std::string getContent() const { return m_ss.str();}
        //日志器
        std::shared_ptr<Logger> getLogger() const { return m_logger;}
        //日志级别
        LogLevel::Level getLevel() const { return m_level;}
        //流
        std::stringstream& getSS() { return m_ss;}
        //格式化写入内容
        void format(const char* fmt, ...);
        //格式化写入内容
        void format(const char* fmt, va_list al);
    private:
        /// 文件名
        const char* m_file = nullptr;
        /// 行号
        int32_t m_line = 0;
        /// 程序启动开始到现在的毫秒数
        uint32_t m_elapse = 0;
        /// 线程ID
        uint32_t m_threadId = 0;
        /// 协程ID
        uint32_t m_fiberId = 0;
        /// 时间戳
        uint64_t m_time = 0;
        /// 线程名称
        std::string m_threadName;
        /// 日志内容流
        std::stringstream m_ss;
        /// 日志器
        std::shared_ptr<Logger> m_logger;
        /// 日志等级
        LogLevel::Level m_level;
    };

    class LogFormatter {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string& pattern);

        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
        std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    public:

        class FormatItem {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem() {}
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        void init();

        bool isError() const { return m_error;}

        const std::string getPattern() const { return m_pattern;}
    private:
        /// 日志格式模板
        std::string m_pattern;
        /// 日志格式解析后格式
        std::vector<FormatItem::ptr> m_items;
        /// 是否有错误
        bool m_error = false;

    };

    class LogAppender {
        friend class Logger;
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender() {}
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        void setFormatter(LogFormatter::ptr val);

        LogFormatter::ptr getFormatter();

        LogLevel::Level getLevel() const { return m_level;}

        void setLevel(LogLevel::Level val) { m_level = val;}
    protected:
        /// 日志级别
        LogLevel::Level m_level = LogLevel::DEBUG;
        /// 是否有自己的日志格式器
        bool m_hasFormatter = false;

        /// 日志格式器
        LogFormatter::ptr m_formatter;
    };

    class Logger : public std::enable_shared_from_this<Logger> {
        friend class LoggerManager;
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string& name = "root");

        void log(LogLevel::Level level, LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);

        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);

        void delAppender(LogAppender::ptr appender);

        void clearAppenders();

        LogLevel::Level getLevel() const { return m_level;}
        void setLevel(LogLevel::Level val) { m_level = val;}
        const std::string& getName() const { return m_name;}
        void setFormatter(LogFormatter::ptr val);
        void setFormatter(const std::string& val);
        LogFormatter::ptr getFormatter();
        std::string toYamlString();
    private:
        /// 日志名称
        std::string m_name;
        /// 日志级别
        LogLevel::Level m_level;
        /// 日志目标集合
        std::list<LogAppender::ptr> m_appenders;
        /// 日志格式器
        LogFormatter::ptr m_formatter;
        /// 主日志器
        Logger::ptr m_root;
    };

    class StdoutLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    };

    class FileLogAppender:public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string& filename);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

        bool reopen();
    private:
        /// 文件路径
        std::string m_filename;
        /// 文件流
        std::ofstream m_filestream;
        /// 上次重新打开时间
        uint64_t m_lastTime = 0;
    };

}

#endif
