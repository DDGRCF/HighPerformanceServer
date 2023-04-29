#ifndef SYLAR_LOG_H_
#define SYLAR_LOG_H_

#include <stdint.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace sylar {

class Logger;

// LogLevel
class LogLevel {
 public:
  enum Level { DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4, FATAL = 5 };

  static const char* ToString(LogLevel::Level level);
};

class LogEvent {
 public:
  LogEvent(const char* file, const int32_t& line, const uint32_t& elapse,
           const uint32_t& thread_id, const uint32_t& fiber_id,
           const uint64_t& time);

  typedef std::shared_ptr<LogEvent> ptr;

  const char* getFile() const { return m_file; };

  const int32_t getLine() const { return m_line; }

  const uint32_t getElapse() const { return m_elapse; }

  const uint32_t getThreadId() const { return m_threadId; }

  const uint32_t getFiberId() const { return m_fiberId; }

  const uint64_t getTime() const { return m_time; }

  const std::string getContent() const { return m_ss.str(); }

  std::stringstream& getSS() { return m_ss; }

 private:
  const char* m_file = nullptr;
  int32_t m_line = 0;
  uint32_t m_elapse = 0;
  uint32_t m_threadId = 0;  // 线程号
  uint32_t m_fiberId = 0;   // 协程号

  uint64_t m_time;

  std::stringstream m_ss;
};

// LogFormatter
class LogFormatter {
 public:
  LogFormatter(const std::string& pattern);

  typedef std::shared_ptr<LogFormatter> ptr;
  std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     LogEvent::ptr event);

  void init();

 public:
  class FormatItem {
   public:
    typedef std::shared_ptr<FormatItem> ptr;

    virtual ~FormatItem() {}

    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger,
                        LogLevel::Level level, LogEvent::ptr event) = 0;

   private:
  };

 private:
  std::string m_pattern;
  std::vector<FormatItem::ptr> m_items;
  bool m_error = false;
};

// LogAppender
class LogAppender {
 public:
  typedef std::shared_ptr<LogAppender> ptr;

  virtual ~LogAppender() {}

  virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   LogEvent::ptr event) = 0;

  const LogFormatter::ptr getFormatter() const { return m_formatter; }

  void setFormatter(LogFormatter::ptr val) { m_formatter = val; }

 protected:
  LogLevel::Level m_level;
  LogFormatter::ptr m_formatter;
};

class StdoutLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<LogAppender> ptr;
  void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
           LogEvent::ptr event) override;
};

class FileLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<FileLogAppender> ptr;
  void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
           LogEvent::ptr event) override;
  bool reopen();

 private:
  std::string m_filename;
  std::ofstream m_filestream;
};

// Logger
class Logger : public std::enable_shared_from_this<
                   Logger> {  // 启动这个后，会自动共享这个
 public:
  typedef std::shared_ptr<Logger> ptr;

  Logger(const std::string& name = "root");

  void log(LogLevel::Level level, LogEvent::ptr event);

  void debug(LogEvent::ptr event);
  void info(LogEvent::ptr event);
  void warn(LogEvent::ptr event);
  void error(LogEvent::ptr event);
  void fatal(LogEvent::ptr event);

  LogLevel::Level getLevel();
  void setLevel(LogLevel::Level level);

  void addAppender(LogAppender::ptr appender);
  void delAppender(LogAppender::ptr appender);

  const std::string getName() const { return m_name; }

  const LogLevel::Level getLevel() const { return m_level; }

  const std::list<LogAppender::ptr>& getAppenders() const {
    return m_appenders;
  }

 private:
  std::string m_name;
  LogLevel::Level m_level;
  std::list<LogAppender::ptr> m_appenders;
  LogFormatter::ptr m_formatter;
};

}  // namespace sylar

#endif
