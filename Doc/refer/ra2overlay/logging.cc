#include <strsafe.h>

#include <filesystem>
namespace fs = std::filesystem;
#include <fstream>
#include <iostream>
#include <stacktrace>

#include "windows_shit.h"
#define EAT_SHIT_FIRST  // prevent linter move windows shit down
#include "logging.h"

namespace ra2overlay {

namespace {
static std::ostringstream kNullStream;
static LogTarget g_log_target = LogTarget::kStandard;
static std::ofstream g_file_sink;
static std::function<void(std::string_view)> g_custom_sink = nullptr;
// verbose log level: display the level that GE g_vlog_level
static LogLevel g_vlog_level = LOG_LVL_INFO;

void Exit(int last_err) {
#ifdef _WIN32
  ExitProcess(last_err);
#else
  exit(last_err);
#endif
}
}  // namespace

namespace debug {
std::string DumpStackTrace() {
  std::stringstream strstream;
  strstream << std::stacktrace::current();
  return strstream.str();
}
}  // namespace debug

void InitLogging(LogTarget log_target, std::string_view log_file) {
  kNullStream.setstate(std::ios_base::badbit);
  g_log_target = log_target;
  if (g_log_target == LogTarget::kFile) {
    if (!g_file_sink.is_open()) {
      g_file_sink = std::ofstream(log_file.data());
    }
  } else {
    if (g_file_sink.is_open()) {
      g_file_sink.close();
    }
  }
}

void InitLogging(std::function<void(std::string_view)> sink) {
  InitLogging(LogTarget::kCustom_);
  g_custom_sink = sink;
  if (g_custom_sink == nullptr) {
    std::cerr << "Invalid custom log sink\n";
    Exit(-1);
  }
}

void SetLogLevel(LogLevel lvl) {
  g_vlog_level = lvl;
}

bool enable_vlog_level(LogLevel lvl) { return g_vlog_level <= lvl; }
std::ostringstream& get_nullstream() { return kNullStream; }

Logging::Logging(const char* file, int line, LogLevel lvl) : lvl_(lvl) {
  time_t raw;
  time(&raw);
  tm t;
  localtime_s(&t, &raw);

  stream() << std::format("{}", LOG_LVL_HINT[lvl])
           << std::format("{:02d}{:02d} {:02d}:{:02d}:{:02d} ", t.tm_mon,
                          t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)
           << std::format("{}:{}] ", file, line);
}

Logging::~Logging() {
  std::string message = stream_.str();
  if (lvl_ != LOG_LVL_FATAL) {
    PrintDebugString(message);
  } else {
    std::error_code ec = std::error_code(errno, std::system_category());
    // "Message [ErrCategory|ErrCode] ErrMessage\n"
    // "StackTrace..."
    PrintDebugString(std::format("{}  [{}|{}] {}\n{}", message,
                                 ec.category().name(), ec.value(), ec.message(),
                                 debug::DumpStackTrace()));
    Exit(ec.value());
  }
}

void Logging::PrintDebugString(const std::string& message) {
  if (g_log_target == LogTarget::kCustom_) {
    g_custom_sink(message);
    return;
  }
  // lock if using multi-threads
#ifdef _WIN32
  if (g_log_target == LogTarget::kWindowsSink) {
    // debug view would append '\n' automatically
    OutputDebugStringA(message.c_str());
    return;
  }
#endif
  if (g_log_target == LogTarget::kStandard) {
    fprintf(stdout, "%s\n", message.c_str());
    fflush(stdout);
  } else if (g_log_target == LogTarget::kFile) {
    g_file_sink << message << "\n";
    if (lvl_ >= LOG_LVL_WARN) [[unlikely]] {
      g_file_sink.flush();
    }
  } else {
    std::cerr << "Invalid log target=" << static_cast<int>(g_log_target)
              << "\n";
  }
}

}  // namespace ra2overlay
