//
// Created by 王泓哲 on 03/07/2022.
//

#include "log.h"
#include <map>
#include <iostream>
#include <functional>
#include <time.h>
#include "util.h"

namespace euterpe {

    /// 用于输出level的字符串
    const char* LogLevel::ToString(LogLevel::Level level) {
        switch(level) {
#define XX(name) \
case LogLevel::name: \
return #name; \
break;

            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
#undef XX
            default:
                return "UNKNOW";
        }
        return "UNKNOW";
    }

    LogLevel::Level LogLevel::FromString(const std::string& str) {
#define XX(level, v) \
if(str == #v) { \
return LogLevel::level; \
}
        XX(DEBUG, debug);
        XX(INFO, info);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);

        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
        return LogLevel::UNKNOW;
#undef XX
    }

    LogEvent::~LogEvent() {}

    /// 用于记录log event的内容(中发生了什么记录在m_ss中)
    void LogEvent::format(const char* fmt, ...) {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    /// 用于记录log event的内容(中发生了什么记录在m_ss中)
    void LogEvent::format(const char* fmt, va_list al) {
        char* buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if(len != -1) {
            m_ss << std::string(buf, len);
            free(buf);
        }
    }

    LogEventWrap::LogEventWrap(LogEvent::ptr e):m_event(e){

    };

    LogEventWrap::~LogEventWrap(){
        m_event->getLogger()->log(m_event->getLevel(),m_event);
    };

    std::stringstream& LogEventWrap::getSS(){
        return m_event->getSS();
    };

    /// 修改LogAppender中的格式
    /// 并且返回是否有被修改
    void LogAppender::setFormatter(LogFormatter::ptr val) {
        m_formatter = val;
        if(m_formatter) {
            m_hasFormatter = true;
        } else {
            m_hasFormatter = false;
        }
    }

    /// 获取LogAppender中的Formatter
    LogFormatter::ptr LogAppender::getFormatter() {
        return m_formatter;
    }

    ////////////////////////////////////////////////////////////////
    ///////////////////// 这里是所有的item类 /////////////////////////
    ////////////////////////////////////////////////////////////////
    class MessageFormatItem : public LogFormatter::FormatItem {
    public:
        MessageFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem {
    public:
        LevelFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << LogLevel::ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem {
    public:
        ElapseFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem {
    public:
        NameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLogger()->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadIdFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem {
    public:
        FiberIdFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFiberId();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadNameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getThreadName();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem {
    public:
        DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
            if(m_format.empty()) {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }
    private:
        std::string m_format;
    };

    class FilenameFormatItem : public LogFormatter::FormatItem {
    public:
        FilenameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem {
    public:
        LineFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem {
    public:
        NewLineFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem {
    public:
        StringFormatItem(const std::string& str)
        :m_string(str) {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << m_string;
        }
    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem {
    public:
        TabFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << "\t";
        }
    private:
        std::string m_string;
    };
    ////////////////////////////////////////////////////////////////
    //////////////////////////////结束 //////////////////////////////
    ////////////////////////////////////////////////////////////////

    /// logEvent的构造函数
    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
                       ,const char* file, int32_t line, uint32_t elapse
                       ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
                       ,const std::string& thread_name)
                       :m_file(file)
                       ,m_line(line)
                       ,m_elapse(elapse)
                       ,m_threadId(thread_id)
                       ,m_fiberId(fiber_id)
                       ,m_time(time)
                       ,m_threadName(thread_name)
                       ,m_logger(logger)
                       ,m_level(level) {
    }

    /// Logger的构造函数 有名字和等级 有默认记录值 需后续修改
    Logger::Logger(const std::string& name):m_name(name),m_level(LogLevel::DEBUG) {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }

    /// 修改logger中的格式
    void Logger::setFormatter(LogFormatter::ptr val) {
        m_formatter = val;

        for(auto& i : m_appenders) {
            if(!i->m_hasFormatter) {
                i->m_formatter = m_formatter;
            }
        }
    }

    ///用字符串修改logger中的格式
    void Logger::setFormatter(const std::string& val) {
        std::cout << "---" << val << std::endl;
        euterpe::LogFormatter::ptr new_val(new euterpe::LogFormatter(val));
        if(new_val->isError()) {
            std::cout << "Logger setFormatter name=" << m_name
            << " value=" << val << " invalid formatter"
            << std::endl;
            return;
        }
        setFormatter(new_val);
    }

    ///获取logger中的格式
    LogFormatter::ptr Logger::getFormatter() {
        return m_formatter;
    }

    /// 为logger绑定一个输出地址类
    void Logger::addAppender(LogAppender::ptr appender) {

        if(!appender->getFormatter()) {

            appender->m_formatter = m_formatter;
        }
        m_appenders.push_back(appender);
    }

    /// 为logger删除一个地址类
    void Logger::delAppender(LogAppender::ptr appender) {

        for(auto it = m_appenders.begin();
        it != m_appenders.end(); ++it) {
            if(*it == appender) {
                m_appenders.erase(it);
                break;
            }
        }
    }

    ///记录函数的入口
    ///调用这个函数 log就会开始启动
    ///把两个参数传给appender
    ///appender的format类再进行解析并输出
    void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
        if(level >= m_level) {
            auto self = shared_from_this();
            if(!m_appenders.empty()) {
                for(auto& i : m_appenders) {
                    i->log(self, level, event);
                }
            } else if(m_root) {
                m_root->log(level, event);
            }
        }
    }

    ///重新打开一个文件
    FileLogAppender::FileLogAppender(const std::string& filename):m_filename(filename) {
        reopen();
    }

    /// logappender中的log函数 会调用类中的成员变量logformat进行格式解析 并输出
    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
        if(level >= m_level) {
            uint64_t now = event->getTime();
            if(now >= (m_lastTime + 3)) {
                reopen();
                m_lastTime = now;
            }

            //if(!(m_filestream << m_formatter->format(logger, level, event))) {
            if(!m_formatter->format(m_filestream, logger, level, event)) {
                std::cout << "error" << std::endl;
            }
        }
    }

    ///重新打开函数
    bool FileLogAppender::reopen(){
        if(m_filestream){
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        return !!m_filestream;
    }

    /// 和FileLogAppender::log一个道理
    void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
        if(level >= m_level) {
            m_formatter->format(std::cout, logger, level, event);
        }
    }

    /// LogFormatter构造函数
    LogFormatter::LogFormatter(const std::string& pattern):m_pattern(pattern) {
        init();
    }

    /// 然后分别调用Item具体类的输出函数进行输出 这里是以字符串输出
    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
        std::stringstream ss;
        for(auto& i : m_items) {
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }

    /// 然后分别调用Item具体类的输出函数进行输出 这里是以流输出
    std::ostream& LogFormatter::format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
        for(auto& i : m_items) {
            i->format(ofs, logger, level, event);
        }
        return ofs;
    }

    /// 在logformat构造时被执行
    /// 创建所需的子类实例
    //%xxx %xxx{xxx} %%
    void LogFormatter::init() {
        //str, format, type
        std::vector<std::tuple<std::string, std::string, int> > vec;
        std::string nstr;
        for(size_t i = 0; i < m_pattern.size(); ++i) {
            if(m_pattern[i] != '%') {
                nstr.append(1, m_pattern[i]);
                continue;
            }

            if((i + 1) < m_pattern.size()) {
                if(m_pattern[i + 1] == '%') {
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0;

            std::string str;
            std::string fmt;
            while(n < m_pattern.size()) {
                if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                && m_pattern[n] != '}')) {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }
                if(fmt_status == 0) {
                    if(m_pattern[n] == '{') {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        //std::cout << "*" << str << std::endl;
                        fmt_status = 1; //解析格式
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                } else if(fmt_status == 1) {
                    if(m_pattern[n] == '}') {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        //std::cout << "#" << fmt << std::endl;
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if(n == m_pattern.size()) {
                    if(str.empty()) {
                        str = m_pattern.substr(i + 1);
                    }
                }
            }

            if(fmt_status == 0) {
                if(!nstr.empty()) {
                    vec.push_back(std::make_tuple(nstr, std::string(), 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            } else if(fmt_status == 1) {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                m_error = true;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
        }

        if(!nstr.empty()) {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }
        static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C) \
{#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt));}}

                XX(m, MessageFormatItem),           //m:消息
                        XX(p, LevelFormatItem),             //p:日志级别
                        XX(r, ElapseFormatItem),            //r:累计毫秒数
                        XX(c, NameFormatItem),              //c:日志名称
                        XX(t, ThreadIdFormatItem),          //t:线程id
                        XX(n, NewLineFormatItem),           //n:换行
                        XX(d, DateTimeFormatItem),          //d:时间
                        XX(f, FilenameFormatItem),          //f:文件名
                        XX(l, LineFormatItem),              //l:行号
                        XX(T, TabFormatItem),               //T:Tab
                        XX(F, FiberIdFormatItem),           //F:协程id
                        XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX
        };

        for(auto& i : vec) {
            if(std::get<2>(i) == 0) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            } else {
                auto it = s_format_items.find(std::get<0>(i));
                if(it == s_format_items.end()) {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                    m_error = true;
                } else {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }
        }
    }
}