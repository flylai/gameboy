#pragma once

#include <iostream>
#include <sstream>

namespace utils {

[[noreturn]] static void killProcess() { abort(); }

enum class LogLevel {
  kTRACE = 0,
  kDEBUG,
  kINFO,
  kWARN,
  kERROR,
  kUNKNOWN,
};

static inline constexpr const char *getNameFromLogLevel(LogLevel log_level) {
  switch (log_level) {
    case LogLevel::kTRACE:
      return "TRACE";
    case LogLevel::kDEBUG:
      return "DEBUG";
    case LogLevel::kINFO:
      return "INFO";
    case LogLevel::kWARN:
      return "WARN";
    case LogLevel::kERROR:
      return "ERROR";
    case LogLevel::kUNKNOWN:
    default:
      return "UNKNOWN";
  }
}

// This class is used in this case:
// GLOBAL_LOG_LEVEL = TRACE;
// GB_LOG(ERROR) << "COMPILE ERROR";
class LogMessageVoidify {
public:
  void operator&(std::ostream &) {}
};

class LogMessage {
public:
  LogMessage(LogLevel log_level, const char *file, int line)
      : log_level_(log_level),
        file_(file),
        line_(line) {
    stream_ << withColor(true);
    stream_ << "[" << getNameFromLogLevel(log_level) << "] ";
    stream_ << "(" << file_ << ":" << line_ << ") ";
  }

  inline constexpr const char *withColor(bool is_prefix) const {
#ifndef _WIN32
    if (!is_prefix) return "\x1b[0m";
    switch (log_level_) {
      case LogLevel::kTRACE:
        return "\x1b[90m";
      case LogLevel::kDEBUG:
        return "\x1b[32m";
      case LogLevel::kINFO:
        return "\x1b[34m";
      case LogLevel::kWARN:
        return "\x1b[93m";
      case LogLevel::kERROR:
        return "\x1b[31m";
      case LogLevel::kUNKNOWN:
        break;
    }
#endif
    return "";
  }

  ~LogMessage() {
    stream_ << withColor(false) << std::endl;
    if (log_level_ < LogLevel::kWARN) [[likely]] {
      std::cout << stream_.str();
      std::cout.flush();
    } else {
      std::cerr << stream_.str();
      std::cerr.flush();
      if (log_level_ == LogLevel::kERROR) [[unlikely]] {
        killProcess();
      }
    }
  }

  std::ostream &stream() { return stream_; }

private:
  std::ostringstream stream_;
  const LogLevel log_level_;
  const char *file_;
  const int line_;
};

} // namespace utils

//constexpr static utils::LogLevel GLOBAL_LOG_LEVEL = utils::LogLevel::kTRACE;
constexpr static utils::LogLevel GLOBAL_LOG_LEVEL = utils::LogLevel::kINFO;

#define GB_LOG_STREAM(log_level) ::utils::LogMessage(log_level, __FILE__, __LINE__).stream()

#define GB_LOG_IS_ENABLE(log_level) GLOBAL_LOG_LEVEL > (log_level)

#define GB_LAZY_LOG(log_level, stream) \
  GB_LOG_IS_ENABLE(log_level) ? (void) 0 : ::utils::LogMessageVoidify() & (stream)

#define GB_LOG_LEVEL(raw_log_level) ::utils::LogLevel::k##raw_log_level

#define GB_LOG(raw_log_level) \
  GB_LAZY_LOG(GB_LOG_LEVEL(raw_log_level), GB_LOG_STREAM(GB_LOG_LEVEL(raw_log_level)))

#define GB_CHECK(condition) (condition) ? (void) 0 : (GB_LOG(ERROR) << #condition << " is NOT true!!")

#define GB_UNREACHABLE()                           \
  {                                                \
    GB_LOG(ERROR) << "This case is unreachable!!"; \
    ::utils::killProcess();                        \
  }
