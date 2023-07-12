#include "log.h"
#include "config.h"
#include "lexicalcast.h"

namespace ddg {

// LogLevel
std::string LogLevel::ToString(LogLevel::Level level) {
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

LogLevel::Level LogLevel::FromString(const std::string& level) {
  std::string new_level = level;
  std::transform(level.begin(), level.end(), new_level.begin(), ::toupper);
#define XX(name)            \
  if (#name == new_level) { \
    return LogLevel::name;  \
  }
  XX(DEBUG);
  XX(INFO);
  XX(WARN);
  XX(ERROR);
  XX(FATAL);
  return LogLevel::UNKNOW;
#undef XX
}

std::ostream& operator<<(std::ostream& os, const LogLevel::Level level) {
  os << LogLevel::ToString(level);
  return os;
}

std::istream& operator>>(std::istream& is, LogLevel::Level& level) {
  std::string name;
  is >> name;
  level = LogLevel::FromString(name);
  return is;
}

// Logger
Logger::Logger(const std::string& name)
    : m_name(name), m_level(LogLevel::DEBUG) {
  m_formatter.reset(
      new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T<%f:%l>%T%m%n"));
}

Logger::ptr Logger::getRoot() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_root;
}

void Logger::setRoot(Logger::ptr root) {
  RWMutexType::WriteLock lock(m_rwmutex);
  m_root = root;
}

void Logger::setFormatter(const std::string& pattern) {
  auto i = std::make_shared<LogFormatter>(pattern);
  if (i->getError()) {
    std::cout << "Logger::setFormatter name = " << m_name
              << " pattern = " << pattern << " invalid formatter" << std::endl;
    return;
  }
  setFormatter(m_formatter);
}

void Logger::setFormatter(LogFormatter::ptr formatter) {
  for (auto& i : m_appenders) {
    i->setFormatter(formatter);
  }
}

void Logger::addAppender(LogAppender::ptr appender) {
  if (!appender->getFormatter()) {
    appender->setFormatter(m_formatter);
  }
  RWMutexType::WriteLock lock(m_rwmutex);
  m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
  RWMutexType::WriteLock lock(m_rwmutex);
  for (auto it = m_appenders.begin(); it != m_appenders.end();) {
    if (*it ==
        appender) {  // 不要在for循环里删除元素：https://blog.csdn.net/zymill/article/details/79836586
      m_appenders.erase(it++);
    } else {
      it++;
    }
  }
}

void Logger::clearAppender() {
  RWMutexType::WriteLock lock(m_rwmutex);
  m_appenders.clear();
}

void Logger::setLevel(LogLevel::Level level) {
  RWMutexType::WriteLock lock(m_rwmutex);
  m_level = level;
}

LogLevel::Level Logger::getLevel() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_level;
}

const std::string& Logger::getName() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_name;
}

LogFormatter::ptr Logger::getFormatter() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_formatter;
}

const std::list<LogAppender::ptr>& Logger::getAppenders() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_appenders;
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
  RWMutexType::ReadLock lock(m_rwmutex);
  if (level >= m_level) {
    auto self = shared_from_this();
    if (!m_appenders.empty()) {
      for (auto& i : m_appenders) {
        i->log(self, level, event);
      }
    } else {
      m_root->log(level, event);
    }
  }
}

void Logger::debug(LogEvent::ptr event) {
  log(LogLevel::DEBUG, event);
}

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

std::string Logger::toYamlString() const {
  YAML::Node node;
  {
    RWMutexType::ReadLock lock(m_rwmutex);
    node["name"] = m_name;
    if (m_level != LogLevel::UNKNOW) {
      node["level"] = LogLevel::ToString(m_level);
    }
    if (m_formatter) {
      node["formatter"] = m_formatter->getPattern();
    }

    for (auto& i : m_appenders) {
      node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

std::string Logger::toString() const {
  return toYamlString();
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
void LogAppender::setFormatter(LogFormatter::ptr val) {
  RWMutexType::WriteLock lock(m_rwmutex);
  m_formatter = val;
}

LogFormatter::ptr LogAppender::getFormatter() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_formatter;
}

void LogAppender::setLevel(LogLevel::Level level) {
  RWMutexType::WriteLock lock(m_rwmutex);
  m_level = level;
}

std::string StdoutLogAppender::toYamlString() const {
  YAML::Node node;
  {
    RWMutexType::ReadLock lock(m_rwmutex);
    node["level"] = LogLevel::ToString(m_level);
    node["type"] = LogAppender::ToString(LogAppender::STDOUT_LOG_APPENDER);
    node["formatter"] = m_formatter->getPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

std::string StdoutLogAppender::toString() const {
  return toYamlString();
}

std::string FileLogAppender::toYamlString() const {
  YAML::Node node;
  {
    RWMutexType::ReadLock lock(m_rwmutex);
    node["level"] = LogLevel::ToString(m_level);
    node["type"] = LogAppender::ToString(LogAppender::STDOUT_LOG_APPENDER);
    node["formatter"] = m_formatter->getPattern();
    node["file"] = m_filename;
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

std::string FileLogAppender::toString() const {
  return toYamlString();
}

#define FILE_LOG_APPENDER_STR "FileLogAppender"
#define STDOUT_LOG_APPENDER_STR "StdoutLogAppender"

LogAppender::Type LogAppender::FromString(const std::string& type) {
#define XX(name)              \
  if (name##_STR == type) {   \
    return LogAppender::name; \
  }
  XX(FILE_LOG_APPENDER);
  XX(STDOUT_LOG_APPENDER);
  return LogAppender::UNKNOW_APPENDER;

#undef XX
}

std::string LogAppender::ToString(LogAppender::Type type) {
  switch (type) {
#define XX(name)          \
  case LogAppender::name: \
    return name##_STR;    \
    break
    XX(FILE_LOG_APPENDER);
    XX(STDOUT_LOG_APPENDER);
    default:
      return "UnknowAppender";
#undef XX
  }
}

#undef FILE_LOG_APPENDER_STR
#undef STDOUT_LOG_APPENDER_STR

void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level,
                            LogEvent::ptr event) {
  MutexType::Lock lock(m_mutex);
  if (level >= m_level) {
    std::cout << m_formatter->format(logger, level, event);
  }
}

FileLogAppender::FileLogAppender(const std::string& file) : m_filename(file) {
  m_filestream.open(m_filename);
}

void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level,
                          LogEvent::ptr event) {
  MutexType::Lock lock(m_mutex);
  if (level >= m_level) {
    m_filestream << m_formatter->format(logger, level, event);
  }
}

bool FileLogAppender::reopen() {
  RWMutexType::WriteLock lock(m_rwmutex);
  if (m_filestream) {
    m_filestream.close();
  }
  m_filestream.open(m_filename);
  return !m_filestream;
}

// Formatter
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
  m_error = !initPattern(m_pattern, m_items);
}

bool LogFormatter::checkValid(const std::string& pattern) {
  std::vector<FormatItem::ptr> items;
  return LogFormatter::initPattern(pattern, items);
}

bool LogFormatter::initPattern(const std::string& pattern,
                               std::vector<FormatItem::ptr>& items) {
  //str, format, type
  std::vector<std::tuple<std::string, std::string, int>> vec;
  std::string nstr;
  bool error = false;
  for (size_t i = 0; i < pattern.size(); i++) {
    if (pattern[i] != '%') {
      nstr.push_back(pattern[i]);
      continue;
    }

    if (i + 1 < pattern.size()) {
      if (pattern[i + 1] == '%') {
        nstr.push_back('%');
        continue;
      }
    }

    size_t n = i + 1;
    std::string str, fmt;
    uint8_t fmt_status = 0;
    size_t fmt_begin = 0;

    while (n < pattern.size()) {
      if (!fmt_status && !isalpha(pattern[n]) && pattern[n] != '{' &&
          pattern[n] != '}') {
        str = pattern.substr(i + 1, n - i - 1);
        break;
      }

      if (fmt_status == 0) {
        if (pattern[n] == '{') {
          str = pattern.substr(i + 1, n - i - 1);
          fmt_status = 1;
          fmt_begin = n;
          n++;
          continue;
        }
      } else if (fmt_status == 1) {
        if (pattern[n] == '}') {
          fmt = pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
          fmt_status = 0;
          n++;
          break;
        }
      }
      n++;
      if (n == pattern.size()) {
        if (str.empty()) {
          str = pattern.substr(i + 1);
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
      std::cout << "pattern parse error: " << pattern << " - "
                << pattern.substr(i) << std::endl;
      error = true;
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
      items.push_back(std::make_shared<StringFormatItem>(std::get<0>(i)));
    } else {
      auto it = s_format_items.find(std::get<0>(i));
      if (it == s_format_items.end()) {
        items.push_back(std::make_shared<StringFormatItem>(
            "<<error_format %" + std::get<0>(i) + ">>"));
        error = true;
      } else {
        items.push_back(it->second(std::get<1>(i)));
      }
    }
  }
  if (error) {
    throw std::invalid_argument(pattern);
  }
  return !error;
}

bool LogFormatter::setPattern(const std::string& pattern) {
  RWMutexType::WriteLock lock(m_rwmutex);
  m_error = !initPattern(pattern, m_items);
  return !m_error;
}

bool LogFormatter::getError() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_error;
}

std::string LogFormatter::getPattern() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_pattern;
}

std::string LogFormatter::format(Logger::ptr logger, LogLevel::Level level,
                                 LogEvent::ptr event) {
  std::stringstream ss;
  {
    RWMutexType::ReadLock lock(m_rwmutex);
    for (auto& i : m_items) {
      i->format(ss, logger, level, event);
    }
  }
  return ss.str();
}

// Log dataset structure
bool LogAppenderDefine::operator==(const LogAppenderDefine& oth) const {
  return type == oth.type && level == oth.level && formatter == oth.formatter &&
         oth.file == oth.file;
}

bool LogDefine::operator==(const LogDefine& oth) const {
  if (appenders.size() != oth.appenders.size()) {
    return false;
  }

  for (size_t i = 0; i < oth.appenders.size(); i++) {
    if (!(appenders[i] == oth.appenders[i])) {
      return false;
    }
  }

  return name == oth.name && level == oth.level && formatter == oth.formatter;
}

bool LogDefine::operator<(const LogDefine& oth) const {
  return name < oth.name;
}

std::string LogAppenderDefine::getString() const {
  return LexicalCast<LogAppenderDefine, std::string>()(*this);
}

std::string LogDefine::getString() const {
  return LexicalCast<LogDefine, std::string>()(*this);
}

// LoggerManager
LoggerManager::LoggerManager() {
  m_root.reset(new Logger("root"));
  auto appender = std::make_shared<StdoutLogAppender>();

  appender->setLevel(LogLevel::DEBUG);
  m_root->setLevel(LogLevel::DEBUG);

  m_root->addAppender(appender);
  m_loggers[m_root->getName()] = m_root;

  ConfigVar<std::set<LogDefine>>::ptr g_log_defines = ddg::Config::Lookup(
      "logs", std::set<LogDefine>(), "logs config");  // TODO:

  auto func = [](const std::set<LogDefine>& old_defines,
                 const std::set<LogDefine>& new_defines) {
    // 增加
    for (auto& i : new_defines) {
      auto it = old_defines.find(i);
      ddg::Logger::ptr logger;
      if (it == old_defines.end()) {
        logger = DDG_LOG_NAME(i.name);
      } else {
        if (!(i == *it)) {
          logger = DDG_LOG_NAME(i.name);
        } else {
          continue;
        }
      }
      logger->setLevel(i.level);
      if (!i.formatter.empty()) {
        logger->setFormatter(i.formatter);
      }

      logger->clearAppender();
      for (auto& appender : i.appenders) {
        ddg::LogAppender::ptr ap;
        switch (appender.type) {
          case LogAppender::STDOUT_LOG_APPENDER:
            ap.reset(new StdoutLogAppender);
            break;
          case LogAppender::FILE_LOG_APPENDER:
            ap.reset(new FileLogAppender(appender.file));
            break;
          default:
            break;
        }

        ap->setLevel(appender.level);
        if (!appender.formatter.empty()) {
          ap->setFormatter(std::make_shared<LogFormatter>(appender.formatter));
        }
        logger->addAppender(ap);
      }
    }

    for (auto& i : old_defines) {
      auto it = new_defines.find(i);
      if (it == new_defines.end()) {
        auto logger = DDG_LOG_NAME(i.name);
        logger->setLevel(static_cast<LogLevel::Level>(
            100));  // 设置后会立即生效，但是在使用的线程或者协程，正在使用不会报错
        logger->clearAppender();  // 当其他进程和携程使用完后，就可以释放了
      }
    }
  };
  g_log_defines->addListener(func);
}

LoggerManager::~LoggerManager() {}

Logger::ptr LoggerManager::getRoot() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_root;
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
  RWMutexType::WriteLock lock(m_rwmutex);
  if (m_loggers.find(name) != m_loggers.end()) {
    return m_loggers[name];
  }

  Logger::ptr logger = std::make_shared<Logger>(name);
  logger->setRoot(m_root);

  m_loggers[name] = logger;
  return logger;
}

void LoggerManager::delLogger(const std::string& name) {
  RWMutexType::WriteLock lock(m_rwmutex);
  if (m_loggers.find(name) == m_loggers.end()) {
    return;
  }
  m_loggers.erase(name);
}

std::string LoggerManager::toYamlString() const {
  YAML::Node node;
  {
    RWMutexType::ReadLock lock(m_rwmutex);
    for (auto& i : m_loggers) {
      node.push_back(YAML::Load(i.second->toYamlString()));
    }
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

std::string LoggerManager::toString() const {
  return toYamlString();
}

}  // namespace ddg
