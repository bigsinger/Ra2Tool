#pragma once
#include <stdint.h>

#include <format>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>

#include "logging.h"

namespace ra2overlay {

enum class LogTarget {
#ifdef _WIN32
  // To OutputDebugString()
  kWindowsSink,
#endif
  // To stdout and stderr.
  kStandard,
  // To file.
  kFile,
  // To custom sink. Only for internal use.
  kCustom_,
};

typedef int LogLevel;
constexpr static LogLevel LOG_LVL_TRACE = 0;
constexpr static LogLevel LOG_LVL_INFO  = 1;
constexpr static LogLevel LOG_LVL_WARN  = 2;
constexpr static LogLevel LOG_LVL_ERROR = 3;
constexpr static LogLevel LOG_LVL_FATAL = 4;
constexpr static LogLevel LOG_LVL_NULL  = 5;
constexpr const char* LOG_LVL_HINT = "TIWEFN";

// log_file only used when log_target set to LogTarget::kFile.
void InitLogging(LogTarget log_target, std::string_view log_file = "");
// Use custom log function, internally set log_target to LogTarget::kCustom_.
void InitLogging(std::function<void(std::string_view)> sink);
// Set log level.
void SetLogLevel(LogLevel lvl);

namespace debug {
std::string DumpStackTrace();
}  // namespace debug
bool enable_vlog_level(LogLevel lvl);
std::ostringstream& get_nullstream();

class Logging {
 public:
  Logging(const char* file, int line, LogLevel lvl);
  Logging(const Logging&) = delete;
  Logging& operator=(const Logging&) = delete;
  ~Logging();
  std::ostringstream& stream() { return stream_; }

 private:
  LogLevel lvl_;
  std::ostringstream stream_;

  void PrintDebugString(const std::string& message);
};

}  // namespace ra2overlay

constexpr const char* GetLastPathSeparator(const char* path) {
  const char* last_separator = path;
  while (*path != '\0') {
    if (*path == '/' || *path == '\\') {
      last_separator = path + 1;
    }
    path++;
  }
  return last_separator;
}

#define __FILENAME GetLastPathSeparator(__FILE__)

#define GLOG_TRACE                \
  !ra2overlay::enable_vlog_level(0)  \
      ? ra2overlay::get_nullstream() \
      : ra2overlay::Logging(__FILENAME, __LINE__, 0).stream()
#define GLOG_INFO                 \
  !ra2overlay::enable_vlog_level(1)  \
      ? ra2overlay::get_nullstream() \
      : ra2overlay::Logging(__FILENAME, __LINE__, 1).stream()
#define GLOG_WARNING                 \
  !ra2overlay::enable_vlog_level(2)  \
      ? ra2overlay::get_nullstream() \
      : ra2overlay::Logging(__FILENAME, __LINE__, 2).stream()
#define GLOG_ERROR                \
  !ra2overlay::enable_vlog_level(3)  \
      ? ra2overlay::get_nullstream() \
      : ra2overlay::Logging(__FILENAME, __LINE__, 3).stream()
#define GLOG_FATAL                \
  !ra2overlay::enable_vlog_level(4)  \
      ? ra2overlay::get_nullstream() \
      : ra2overlay::Logging(__FILENAME, __LINE__, 4).stream()

#define LOG(lvl) GLOG_##lvl
#define LOG_F(lvl, f, ...) GLOG_##lvl << std::format(f, ##__VA_ARGS__)
#define LOG_IF(lvl, cond) !(cond) ? ra2overlay::get_nullstream() : LOG(lvl)

#define HLOG(lvl)   GLOG_##lvl << get_log_header()
#define HLOG_TRACE  GLOG_TRACE << get_log_header()
#define HLOG_INFO   GLOG_INFO  << get_log_header()
#define HLOG_WARN   GLOG_WARNING  << get_log_header()
#define HLOG_ERROR  GLOG_ERROR << get_log_header()
#define HLOG_FATAL  GLOG_FATAL << get_log_header()
#define HLOG_F(lvl, f, ...) HLOG_##lvl << std::format(f, ##__VA_ARGS__)

#ifdef _DEBUG
#define DLOG_TRACE  GLOG_TRACE
#define DLOG_INFO   GLOG_INFO
#define DLOG_WARN   GLOG_WARNING
#define DLOG_ERROR  GLOG_ERROR
#define DLOG_FATAL  GLOG_FATAL
#define DLOG(lvl)   GLOG_##lvl
#define DLOG_F(lvl, f, ...) DLOG(lvl) << std::format(f, ##__VA_ARGS__)
#define HDLOG(lvl) DLOG_##lvl << get_log_header()
#define HDLOG_F(lvl, f, ...) HDLOG(lvl) << std::format(f, ##__VA_ARGS__)
#else
#define DLOG_TRACE  
#define DLOG_INFO   
#define DLOG_WARN   
#define DLOG_ERROR   
#define DLOG_FATAL  
#define DLOG(lvl)
#define DLOG_F(lvl, f, ...)
#define HDLOG(lvl)
#define HDLOG_F(lvl, f, ...)
#endif

#define CHECK(cond) LOG_IF(FATAL, !(cond)) << "Check failed: " #cond " "
#define CHECK_EQ(a, b) LOG_IF(FATAL,  ((a)!=(b))) << std::format("Check failed: " #a "={} EQ " #b "={} ", (a), (b))
#define CHECK_NE(a, b) LOG_IF(FATAL,  ((a)==(b))) << std::format("Check failed: " #a "={} NE " #b "={} ", (a), (b))
#define CHECK_GE(a, b) LOG_IF(FATAL, !((a)>=(b))) << std::format("Check failed: " #a "={} GE " #b "={} ", (a), (b))
#define CHECK_GT(a, b) LOG_IF(FATAL, !((a)> (b))) << std::format("Check failed: " #a "={} GT " #b "={} ", (a), (b))
#define CHECK_LE(a, b) LOG_IF(FATAL, !((a)<=(b))) << std::format("Check failed: " #a "={} LE " #b "={} ", (a), (b))
#define CHECK_LT(a, b) LOG_IF(FATAL, !((a)< (b))) << std::format("Check failed: " #a "={} LT " #b "={} ", (a), (b))
#define CHECK_F(cond, f, ...) LOG_IF(FATAL, !(cond)) << "Check failed: " #cond " " << std::format(f, ##__VA_ARGS__)
#define CHECK_EQ_F(a, b, f, ...) LOG_IF(FATAL,  ((a)!=(b))) << std::format("Check failed: " #a "={} EQ " #b "={} ", (a), (b)) << std::format(f, ##__VA_ARGS__)
#define CHECK_NE_F(a, b, f, ...) LOG_IF(FATAL,  ((a)==(b))) << std::format("Check failed: " #a "={} NE " #b "={} ", (a), (b)) << std::format(f, ##__VA_ARGS__)
#define CHECK_GE_F(a, b, f, ...) LOG_IF(FATAL, !((a)>=(b))) << std::format("Check failed: " #a "={} GE " #b "={} ", (a), (b)) << std::format(f, ##__VA_ARGS__)
#define CHECK_GT_F(a, b, f, ...) LOG_IF(FATAL, !((a)> (b))) << std::format("Check failed: " #a "={} GT " #b "={} ", (a), (b)) << std::format(f, ##__VA_ARGS__)
#define CHECK_LE_F(a, b, f, ...) LOG_IF(FATAL, !((a)<=(b))) << std::format("Check failed: " #a "={} LE " #b "={} ", (a), (b)) << std::format(f, ##__VA_ARGS__)
#define CHECK_LT_F(a, b, f, ...) LOG_IF(FATAL, !((a)< (b))) << std::format("Check failed: " #a "={} LT " #b "={} ", (a), (b)) << std::format(f, ##__VA_ARGS__)

#ifdef _DEBUG
#define DCHECK(cond)                CHECK(cond)
#define DCHECK_EQ(a, b)             CHECK_EQ(a, b)
#define DCHECK_NE(a, b)             CHECK_NE(a, b)
#define DCHECK_GE(a, b)             CHECK_GE(a, b)
#define DCHECK_GT(a, b)             CHECK_GT(a, b)
#define DCHECK_LE(a, b)             CHECK_LE(a, b)
#define DCHECK_LT(a, b)             CHECK_LT(a, b)
#define DCHECK_F(cond, f, ...)      CHECK_F(cond, f, ##__VA_ARGS__)
#define DCHECK_EQ_F(a, b, f, ...)   CHECK_EQ_F(a, b, f, ##__VA_ARGS__)
#define DCHECK_NE_F(a, b, f, ...)   CHECK_NE_F(a, b, f, ##__VA_ARGS__)
#define DCHECK_GE_F(a, b, f, ...)   CHECK_GE_F(a, b, f, ##__VA_ARGS__)
#define DCHECK_GT_F(a, b, f, ...)   CHECK_GT_F(a, b, f, ##__VA_ARGS__)
#define DCHECK_LE_F(a, b, f, ...)   CHECK_LE_F(a, b, f, ##__VA_ARGS__)
#define DCHECK_LT_F(a, b, f, ...)   CHECK_LT_F(a, b, f, ##__VA_ARGS__)
#else
#define DCHECK(cond)
#define DCHECK_EQ(a, b)
#define DCHECK_NE(a, b)
#define DCHECK_GE(a, b)
#define DCHECK_GT(a, b)
#define DCHECK_LE(a, b)
#define DCHECK_LT(a, b)
#define DCHECK_F(cond, f, ...)
#define DCHECK_EQ_F(a, b, f, ...)
#define DCHECK_NE_F(a, b, f, ...)
#define DCHECK_GE_F(a, b, f, ...)
#define DCHECK_GT_F(a, b, f, ...)
#define DCHECK_LE_F(a, b, f, ...)
#define DCHECK_LT_F(a, b, f, ...)
#endif

#define CHECK_NOTNULL(var) CHECK(var != nullptr)
#ifdef _DEBUG
#define DCHECK_NOTNULL(var) DCHECK(var != nullptr)
#else
#define DCHECK_NOTNULL(var) (var)
#endif

#ifndef UNREACHABLE
#ifdef _WIN32
#define UNREACHABLE() LOG(FATAL) << "unreachable"; __assume(false)
#elif __linux__
#define UNREACHABLE() LOG(FATAL) << "unreachable"; __builtin_unreachable()
#else
#error unknown platform
#endif
#endif

#ifndef NOT_IMPLEMENTED
#define NOT_IMPLEMENTED() LOG(FATAL) << "not implemented"; abort()
#endif

