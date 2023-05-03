#include "log.h"

namespace ddg {

// LogLevel
LogLevel::Level Logger::getLevel() {
  return m_level;
}

const char* LogLevel::ToString(LogLevel::Level level) {

  switch (level) {
#define XX(name)       \
  case LogLevel::name: \
    return #name;      \
    break
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

LogLevel::Level FromString(const std::string& level) {
#define XX(name)           \
  if (#name == level) {    \
    return LogLevel::name; \
  }
  XX(DEBUG);
  XX(INFO);
  XX(WARN);
  XX(ERROR);
  XX(FATAL);
  return LogLevel::UNKNOW;
#undef XX
}

// Logger
Logger::Logger(const std::string& name)
    : m_name(name), m_level(LogLevel::DEBUG) {
  m_formatter.reset(
      new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T<%f:%l>%T%m%n"));
}

void Logger::setLevel(LogLevel::Level level) {
  m_level = level;
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
  if (level >= m_level) {
    auto self = shared_from_this();
    for (auto& i : m_appenders) {
      i->log(self, level, event);
    }
  }
}

void Logger::debug(LogEvent::ptr event) {}

void Logger::info(LogEvent::ptr event) {
  log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
  log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event) {
  log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event) {
  log(LogLevel::FATAL, event);
}

void Logger::addAppender(LogAppender::ptr appender) {
  if (!appender->getFormatter()) {
    appender->setFormatter(m_formatter);
  }
  m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
  for (auto it = m_appenders.begin(); it != m_appenders.end();) {
    if (*it ==
        appender) {  // 不要在for循环里删除元素：https://blog.csdn.net/zymill/article/details/79836586
      m_appenders.erase(it++);
    } else {
      it++;
    }
  }
}

// LogEvent

LogEvent::LogEvent(Logger::ptr logger, LogLevel::Level level, const char* file,
                   const int32_t& line, const uint64_t& elapse,
                   const uint64_t& thread_id, const uint64_t& fiber_id,
                   const uint64_t& time)
    : m_logger(logger),
      m_level(level),
      m_file(file),
      m_line(line),
      m_elapse(elapse),
      m_threadId(thread_id),
      m_fiberId(fiber_id),
      m_time(time) {}

void LogEvent::format(const char* fmt, ...) {
  va_list al;
  va_start(al, fmt);
  format(fmt, al);
  va_end(al);
}

void LogEvent::format(const char* fmt, va_list al) {
  char* buf = nullptr;
  int len = vasprintf(&buf, fmt, al);
  if (len != -1) {
    m_ss << std::string(buf, len);
    free(buf);
  }
}

// LogEventWrap

LogEventWrap::LogEventWrap(LogEvent::ptr event) : m_event(event) {}

LogEventWrap::~LogEventWrap() {
  m_event->getLogger()->log(m_event->getLevel(), m_event);
}

std::stringstream& LogEventWrap::getSS() {
  return m_event->getSS();
}

// Appender
void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level,
                            LogEvent::ptr event) {
  if (level >= m_level) {
    std::cout << m_formatter->format(logger, level, event);
  }
}

void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level,
                          LogEvent::ptr event) {
  if (level >= m_level) {
    m_filestream << m_formatter->format(logger, level, event);
  }
}

bool FileLogAppender::reopen() {
  if (m_filestream) {
    m_filestream.close();
  }
  m_filestream.open(m_filename);
  return !m_filestream;
}

// Formatter
std::string LogFormatter::format(Logger::ptr logger, LogLevel::Level level,
                                 LogEvent::ptr event) {
  std::stringstream ss;
  for (auto& i : m_items) {
    i->format(ss, logger, level, event);
  }
  return ss.str();
}

class StringFormatItem : public LogFormatter::FormatItem {
 public:
  StringFormatItem(const std::string& str) : m_string(str) {}

  const std::string getString() const { return m_string; }

  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << m_string;
  }

 private:
  std::string m_string;
};

// FormatterItems
class MessageFormatItem : public LogFormatter::FormatItem {
 public:
  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getContent();
  }
};

class LevelFormatItem : public LogFormatter::FormatItem {
 public:
  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << LogLevel::ToString(level);
  }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
 public:
  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getElapse();  // TODO:
  }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
 public:
  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getThreadId();
  }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
 public:
  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getFiberId();
  }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
 public:
  DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
      : m_format(format) {
    if (m_format.empty()) {
      m_format = "%Y-%m-%d %H:%M:%S";
    }
  }

  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    struct tm tm;
    time_t time = event->getTime();
    localtime_r(&time, &tm);
    char buf[64]{0};
    strftime(buf, sizeof(buf), m_format.c_str(), &tm);
    os << buf;
  }

 private:
  std::string m_format;
};

class LineFormatItem : public LogFormatter::FormatItem {
 public:
  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getLine();
  }
};

class FilenameFormatItem : public LogFormatter::FormatItem {
 public:
  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getFile();
  }
};

class NameFormatItem : public LogFormatter::FormatItem {
 public:
  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    // os << logger->getName();
    os << event->getLogger()->getName();
  }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
 public:
  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << '\n';
  }
};

class TabFormatItem : public LogFormatter::FormatItem {
 public:
  void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << '\t';
  }
};

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {
  init();
}

void LogFormatter::init() {
  //str, format, type
  std::vector<std::tuple<std::string, std::string, int>> vec;
  std::string nstr;
  // TODO:
  for (size_t i = 0; i < m_pattern.size(); i++) {
    if (m_pattern[i] != '%') {
      nstr.push_back(m_pattern[i]);
      continue;
    }

    if (i + 1 < m_pattern.size()) {
      if (m_pattern[i + 1] == '%') {
        nstr.push_back('%');
        continue;
      }
    }

    size_t n = i + 1;
    std::string str, fmt;
    uint8_t fmt_status = 0;
    size_t fmt_begin = 0;

    while (n < m_pattern.size()) {
      if (!fmt_status && !isalpha(m_pattern[n]) && m_pattern[n] != '{' &&
          m_pattern[n] != '}') {
        str = m_pattern.substr(i + 1, n - i - 1);
        break;
      }

      if (fmt_status == 0) {
        if (m_pattern[n] == '{') {
          str = m_pattern.substr(i + 1, n - i - 1);
          fmt_status = 1;
          fmt_begin = n;
          n++;
          continue;
        }
      } else if (fmt_status == 1) {
        if (m_pattern[n] == '}') {
          fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
          fmt_status = 0;
          n++;
          break;
        }
      }
      n++;
      if (n == m_pattern.size()) {
        if (str.empty()) {
          str = m_pattern.substr(i + 1);
        }
      }
    }

    if (fmt_status == 0) {
      if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, std::string(), 0));
        nstr.clear();
      }
      vec.push_back(std::make_tuple(str, fmt, 1));
      i = n - 1;
    } else if (fmt_status == 1) {
      std::cout << "pattern parse error: " << m_pattern << " - "
                << m_pattern.substr(i) << std::endl;
      m_error = true;
      vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
    }
  }

  if (!nstr.empty()) {
    vec.push_back(std::make_tuple(nstr, "", 0));
  }

  static std::unordered_map<
      std::string, std::function<FormatItem::ptr(const std::string& str)>>
      s_format_items = {
#define XX(str, C)                     \
  {                                    \
#str, [](const std::string& fmt) { \
      return std::make_shared<C>();    \
    }                                  \
  }
          XX(m, MessageFormatItem),
          XX(p, LevelFormatItem),
          XX(r, ElapseFormatItem),
          XX(c, NameFormatItem),
          XX(t, ThreadIdFormatItem),
          XX(n, NewLineFormatItem),
          XX(d, DateTimeFormatItem),
          XX(f, FilenameFormatItem),
          XX(l, LineFormatItem),
          XX(T, TabFormatItem),
          XX(F, FiberIdFormatItem),
          XX(N, ThreadIdFormatItem)
#undef XX
      };
  for (auto& i : vec) {
    if (std::get<2>(i) == 0) {
      m_items.push_back(std::make_shared<StringFormatItem>(std::get<0>(i)));
    } else {
      auto it = s_format_items.find(std::get<0>(i));
      if (it == s_format_items.end()) {
        m_items.push_back(std::make_shared<StringFormatItem>(
            "<<error_format %" + std::get<0>(i) + ">>"));
        m_error = true;
      } else {
        m_items.push_back(it->second(std::get<1>(i)));
      }
    }
  }
}

// MutexLock

class MutexLock::LockGuard {
 public:
  LockGuard(MutexLock& mutex) : m_mutex(mutex) { m_mutex.lock(); }

  LockGuard(const LockGuard&) = delete;  // 拷贝构造函数

  void operator=(const LockGuard&) = delete;  // 拷贝赋值函数

  ~LockGuard() { m_mutex.unlock(); }

 private:
  MutexLock& m_mutex;
};

void NormLock::lock() {
  pthread_mutex_lock(&m_mutex);
}

void NormLock::unlock() {
  pthread_mutex_unlock(&m_mutex);
}

// LoggerManager
LoggerManager::LoggerManager() {
  m_root.reset(new Logger("root"));
  auto appender = std::make_shared<StdoutLogAppender>();

  appender->setLevel(LogLevel::DEBUG);
  m_root->setLevel(LogLevel::DEBUG);

  m_root->addAppender(appender);
  m_loggers[m_root->getName()] = m_root;
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
  MutexLock::LockGuard lock_guard(m_mutex);
  if (m_loggers.find(name) != m_loggers.end()) {
    return m_loggers[name];
  }
  Logger::ptr logger = std::make_shared<Logger>(name);
  return logger;
}

void LoggerManager::init() {}

}  // namespace ddg
