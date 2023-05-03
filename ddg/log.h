#ifndef DDG_LOG_H_
#define DDG_LOG_H_

#include <stdint.h>
#include <time.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "ddg/singleton.h"
#include "ddg/utils.h"
#include "stdarg.h"

// Level
// Logger(DEBUG) -> LogAppender(DEBUG) -> LogEvent(DEBUG)
// if you want to output level, must clear the each level

#define DDG_LOG_LEVEL(logger, level)                                     \
  if (logger->getLevel() <= level)                                       \
  ddg::LogEventWrap(std::make_shared<ddg::LogEvent>(                     \
                        logger, level, __FILE__, __LINE__, 0,            \
                        ddg::getThreadId(), ddg::getFiberId(), time(0))) \
      .getSS()

#define DDG_LOG_DEBUG(logger) DDG_LOG_LEVEL(logger, ddg::LogLevel::DEBUG)

#define DDG_LOG_INFO(logger) DDG_LOG_LEVEL(logger, ddg::LogLevel::INFO)

#define DDG_LOG_WARN(logger) DDG_LOG_LEVEL(logger, ddg::LogLevel::WARN)

#define DDG_LOG_ERROR(logger) DDG_LOG_LEVEL(logger, ddg::LogLevel::ERROR)

#define DDG_LOG_FATAL(logger) DDG_LOG_LEVEL(logger, ddg::LogLevel::FATAL)

#define DDG_LOG_FMT_LEVEL(logger, level, fmt, ...)                       \
  if (logger->getLevel() <= level)                                       \
  ddg::LogEventWrap(std::make_shared<ddg::LogEvent>(                     \
                        logger, level, __FILE__, __LINE__, 0,            \
                        ddg::getThreadId(), ddg::getFiberId(), time(0))) \
      .getEvent()                                                        \
      ->format(fmt, __VA_ARGS__)

#define DDG_LOG_FMT_DEBUG(logger, fmt, ...) \
  DDG_LOG_FMT_LEVEL(logger, ddg::LogLevel::DEBUG, fmt, __VA_ARGS__)

#define DDG_LOG_FMT_INFO(logger, fmt, ...) \
  DDG_LOG_FMT_LEVEL(logger, ddg::LogLevel::INFO, fmt, __VA_ARGS__)

#define DDG_LOG_FMT_WARN(logger, fmt, ...) \
  DDG_LOG_FMT_LEVEL(logger, ddg::LogLevel::WARN, fmt, __VA_ARGS__)

#define DDG_LOG_FMT_ERROR(logger, fmt, ...) \
  DDG_LOG_FMT_LEVEL(logger, ddg::LogLevel::ERROR, fmt, __VA_ARGS__)

#define DDG_LOG_FMT_FATAL(logger, fmt, ...) \
  DDG_LOG_FMT_LEVEL(logger, ddg::LogLevel::FATAL, fmt, __VA_ARGS__)

#define DDG_LOG_VECTOR_DEBUG(logger, vec) \
  {                                       \
    std::string res = "";                 \
    for (auto& i : vec) {                 \
      if (res.empty()) {                  \
        res = std::to_string(i);          \
      } else {                            \
        res += ", " + std::to_string(i);  \
      }                                   \
    }                                     \
    DDG_LOG_DEBUG(logger) << res;         \
  }

#define DDG_LOG_MAP_DEBUG(logger, map)                            \
  {                                                               \
    std::string res = "";                                         \
    for (auto& i : map) {                                         \
      if (res.empty()) {                                          \
        res = i.first + ", " + std::to_string(i.second);          \
      } else {                                                    \
        res += " | " + i.first + ", " + std::to_string(i.second); \
      }                                                           \
    }                                                             \
    DDG_LOG_DEBUG(logger) << res;                                 \
  }

#define DDG_LOG_ROOT() ddg::LoggerMgr::GetInstance()->getRoot()

#define DDG_LOG_NAME(name) ddg::LoggerMgr::GetInstance()->getLogger(name)

namespace ddg {

class Logger;

// LogLevel
class LogLevel {
 public:
  enum Level {
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
    UNKNOW = 6,
  };

  static const char* ToString(LogLevel::Level level);
  static LogLevel::Level FromString(const std::string& level_str);
};

// LogEvent
class LogEvent {
 public:
  LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
           const char* file, const int32_t& line, const uint64_t& elapse,
           const uint64_t& thread_id, const uint64_t& fiber_id,
           const uint64_t& time);

  typedef std::shared_ptr<LogEvent> ptr;

  std::shared_ptr<Logger> getLogger() const { return m_logger; }

  LogLevel::Level getLevel() const { return m_level; }

  const char* getFile() const { return m_file; };

  int32_t getLine() const { return m_line; }

  uint64_t getElapse() const { return m_elapse; }

  uint64_t getThreadId() const { return m_threadId; }

  uint64_t getFiberId() const { return m_fiberId; }

  uint64_t getTime() const { return m_time; }

  std::string getContent() const { return m_ss.str(); }

  void format(const char* fmt, ...);
  void format(const char* fmt, va_list al);

  std::stringstream& getSS() { return m_ss; }

 private:
  std::shared_ptr<Logger> m_logger;
  LogLevel::Level m_level = LogLevel::DEBUG;
  const char* m_file = nullptr;
  int32_t m_line = 0;
  uint64_t m_elapse = 0;
  uint64_t m_threadId = 0;  // 线程号
  uint64_t m_fiberId = 0;   // 协程号

  uint64_t m_time = 0;
  std::stringstream m_ss;
};

// LogEventWrap

class LogEventWrap {
 public:
  LogEventWrap(LogEvent::ptr event);

  ~LogEventWrap();

  std::stringstream& getSS();

  LogEvent::ptr getEvent() const { return m_event; }

 private:
  LogEvent::ptr m_event;
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

  LogFormatter::ptr getFormatter() const { return m_formatter; }

  void setFormatter(LogFormatter::ptr val) { m_formatter = val; }

  void setLevel(LogLevel::Level level) { m_level = level; }

 protected:
  LogLevel::Level m_level = LogLevel::DEBUG;
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

  std::string getName() const { return m_name; }

  LogLevel::Level getLevel() const { return m_level; }

  const std::list<LogAppender::ptr>& getAppenders() const {
    return m_appenders;
  }

 private:
  std::string m_name;
  LogLevel::Level m_level;
  std::list<LogAppender::ptr> m_appenders;
  LogFormatter::ptr m_formatter;
};

class MutexLock {
 public:
  typedef std::shared_ptr<MutexLock> ptr;

  class LockGuard;

  virtual ~MutexLock() {}

  virtual void lock() = 0;
  virtual void unlock() = 0;
};

class NormLock : public MutexLock {
 public:
  typedef pthread_mutex_t LockType;
  void lock() override;
  void unlock() override;

 private:
  LockType m_mutex;
};

// class SpinLock : public MutexLock {};

class LoggerManager {
 public:
  typedef NormLock MutexType;

  LoggerManager();

  Logger::ptr getRoot() const { return m_root; }

  Logger::ptr getLogger(const std::string& name);

  void init();  // extra init for expanding

  std::unordered_map<std::string, Logger::ptr> m_logger;

  std::string toYamlString();

 private:
  MutexType m_mutex;
  Logger::ptr m_root;
  std::unordered_map<std::string, Logger::ptr> m_loggers;
};

typedef Singleton<LoggerManager> LoggerMgr;

}  // namespace ddg

#endif
