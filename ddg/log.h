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
#include "lexicalcast.h"
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

#define DDG_LOG_ROOT() ddg::LoggerMgr::GetInstance()->getRoot()

#define DDG_LOG_NAME(name) ddg::LoggerMgr::GetInstance()->getLogger(name)

#define DDG_LOG_REMOVE(name) ddg::LoggerMgr::GetInstance()->delLogger(name)

namespace ddg {

class Logger;
class LoggerManager;

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
  class FormatItem {
   public:
    typedef std::shared_ptr<FormatItem> ptr;

    virtual ~FormatItem() {}

    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger,
                        LogLevel::Level level, LogEvent::ptr event) = 0;

   private:
  };

 public:
  LogFormatter(const std::string& pattern);

  typedef std::shared_ptr<LogFormatter> ptr;
  std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     LogEvent::ptr event);

  bool getError() const { return m_error; }

  std::string getPattern() const { return m_pattern; }

  static bool checkValid(const std::string& pattern);
  static bool initPattern(const std::string& pattern,
                          std::vector<FormatItem::ptr>& items);

 private:
  std::string m_pattern;
  std::vector<FormatItem::ptr> m_items;
  bool m_error = false;
};

// LogAppender
class LogAppender {
 public:
  enum Type {
    FILE_LOG_APPENDER = 0,
    STDOUT_LOG_APPENDER = 1,
    UNKNOW_APPENDER = 2,
  };

  static LogAppender::Type FromString(const std::string& type);
  static std::string ToString(LogAppender::Type type);

  typedef std::shared_ptr<LogAppender> ptr;

  virtual ~LogAppender() {}

  virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   LogEvent::ptr event) = 0;

  LogFormatter::ptr getFormatter() const { return m_formatter; }

  void setFormatter(LogFormatter::ptr val) { m_formatter = val; }

  void setLevel(LogLevel::Level level) { m_level = level; }

  virtual std::string toYamlString() = 0;

  virtual std::string toString() = 0;

 protected:
  LogLevel::Level m_level = LogLevel::DEBUG;
  LogFormatter::ptr m_formatter;
};

class StdoutLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<LogAppender> ptr;
  void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
           LogEvent::ptr event) override;

  std::string toYamlString() override;
  std::string toString() override;
};

class FileLogAppender : public LogAppender {
 public:
  FileLogAppender(const std::string& file = "/tmp/ddg_server.txt");
  typedef std::shared_ptr<FileLogAppender> ptr;
  void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
           LogEvent::ptr event) override;
  bool reopen();

  std::string toYamlString() override;

  std::string toString() override;

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

  Logger::ptr getRoot() const;
  void setRoot(Logger::ptr root);

  void addAppender(LogAppender::ptr appender);
  void delAppender(LogAppender::ptr appender);
  void clearAppender();

  std::string getName() const { return m_name; }

  void setLevel(LogLevel::Level level);

  LogLevel::Level getLevel() const { return m_level; }

  void setFormatter(LogFormatter::ptr formatter);
  void setFormatter(const std::string& pattern);

  LogFormatter::ptr getFormatter() const { return m_formatter; }

  std::string toYamlString();

  std::string toString();

  const std::list<LogAppender::ptr>& getAppenders() const {
    return m_appenders;
  }

 private:
  std::string m_name;
  LogLevel::Level m_level;
  std::list<LogAppender::ptr> m_appenders;
  LogFormatter::ptr m_formatter;
  Logger::ptr m_root = nullptr;
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

  void delLogger(const std::string& name);

  void init();  // extra init for expanding

  std::unordered_map<std::string, Logger::ptr> m_logger;

  std::string toYamlString();

  std::string toString();

 private:
  MutexType m_mutex;
  Logger::ptr m_root;
  std::unordered_map<std::string, Logger::ptr> m_loggers;
};

// Log dataset structure
struct LogAppenderDefine {
  LogAppender::Type type = LogAppender::UNKNOW_APPENDER;
  LogLevel::Level level = LogLevel::UNKNOW;
  std::string formatter;
  std::string file;

  bool operator==(const LogAppenderDefine& oth) const;

  std::string getString() const;
  // friend std::ostream& operator<<(std::ostream& os, const LogAppenderDefine& define);
};

struct LogDefine {
  std::string name;
  LogLevel::Level level = LogLevel::UNKNOW;
  std::string formatter;
  std::vector<LogAppenderDefine> appenders;

  bool operator==(const LogDefine& oth) const;
  bool operator<(const LogDefine& oth) const;

  std::string getString() const;
  // friend std::ostream& operator<<(std::ostream& os, const LogDefine& define);
};

typedef Singleton<LoggerManager> LoggerMgr;

// LexicalCast
template <>
class LexicalCast<LogAppenderDefine, std::string> {
 public:
  std::string operator()(const LogAppenderDefine& in) {
    YAML::Node node;
    if (in.level != LogLevel::UNKNOW) {
      node["level"] = LogLevel::ToString(in.level);
    }
    if (!in.formatter.empty()) {
      node["formatter"] = in.formatter;
    }

    if (!in.file.empty()) {
      node["file"] = in.file;
    }

    if (in.type != LogAppender::UNKNOW_APPENDER) {
      node["type"] = LogAppender::ToString(in.type);
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <>
class LexicalCast<std::string, LogAppenderDefine> {
 public:
  LogAppenderDefine operator()(const std::string& in) {
    YAML::Node node = YAML::Load(in);
    LogAppenderDefine define;
    for (auto it = node.begin(); it != node.end(); it++) {
      std::string key = it->first.Scalar();
      if (key == "type") {
        define.type = LogAppender::FromString(it->second.Scalar());
      } else if (key == "level") {
        define.level = LogLevel::FromString(it->second.Scalar());
      } else if (key == "file") {
        define.file = it->second.Scalar();
      } else if (key == "formatter") {
        define.formatter = it->second.Scalar();
      } else {
        DDG_LOG_WARN(DDG_LOG_ROOT()) << "LexicalCast(from std::string to "
                                        "LogAppenerDefine) gets unexpected key "
                                     << key;
      }
    }
    return define;
  }
};

template <>
class LexicalCast<LogDefine, std::string> {
 public:
  std::string operator()(const LogDefine& in) {
    YAML::Node node;
    node["name"] = in.name;
    if (in.level != LogLevel::UNKNOW) {
      node["level"] = LogLevel::ToString(in.level);
    }
    if (!in.formatter.empty()) {
      if (!LogFormatter::checkValid(in.formatter)) {
        DDG_LOG_ERROR(DDG_LOG_ROOT())
            << "LexicalCast(from LogDefine to std::string) gets invalid "
               "formatter = "
            << in.formatter;
        node["formatter"] = "";
      } else {
        node["formatter"] = in.formatter;
      }
    }

    for (auto& appender : in.appenders) {
      node["appenders"].push_back(
          YAML::Load(LexicalCast<LogAppenderDefine, std::string>()(appender)));
    }

    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <>
class LexicalCast<std::string, LogDefine> {
 public:
  LogDefine operator()(const std::string& in) {
    YAML::Node node = YAML::Load(in);
    LogDefine define;
    for (auto it = node.begin(); it != node.end(); it++) {
      std::string key = it->first.Scalar();
      if (key == "name") {
        if (it->second.Scalar().empty()) {
          throw std::invalid_argument(key);
        }
        define.name = it->second.Scalar();
      } else if (key == "level") {
        define.level = LogLevel::FromString(it->second.Scalar());
      } else if (key == "formatter") {
        auto formatter = it->second.Scalar();
        if (!LogFormatter::checkValid(formatter)) {
          DDG_LOG_ERROR(DDG_LOG_ROOT())
              << "LexicalCast(from std::string, LogDefine) gets invalid "
                 "formatter = "
              << formatter;
          define.formatter = "";
        } else {
          define.formatter = formatter;
        }
      } else if (key == "appenders") {
        std::stringstream ss;
        for (auto it = node[key].begin(); it != node[key].end(); it++) {
          ss.str("");
          ss.clear();
          ss << *it;
          define.appenders.push_back(
              LexicalCast<std::string, LogAppenderDefine>()(ss.str()));
        }
      } else {
        DDG_LOG_WARN(DDG_LOG_ROOT())
            << "LexicalCast(from std::string, LogDefine) gets unexpected key "
            << key;
      }
    }

    for (auto& appender : define.appenders) {
      if (!define.formatter.empty() && appender.formatter.empty()) {
        appender.formatter = define.formatter;
      }

      if (define.level != LogLevel::UNKNOW &&
          appender.level == LogLevel::UNKNOW) {
        appender.level = define.level;
      }
    }

    return define;
  }
};

}  // namespace ddg

#endif
