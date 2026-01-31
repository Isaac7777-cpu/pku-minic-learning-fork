#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Helper function to get current time as string
inline std::string GetTimestamp() {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::string ts = std::ctime(&t);
  ts.pop_back(); // Remove newline character
  return ts;
}

// A simple logging function that the macro calls
inline void LogFunction(const std::string &level, const std::string &file,
                        int line, const std::string &message) {
  std::stringstream ss;
  ss << GetTimestamp() << " [" << level << "] " << file << ":" << line << " "
     << message;

  if (level == "INFO") {
    std::cout << ss.str() << std::endl;
  } else if (level == "ERROR") {
    throw std::runtime_error(ss.str());
  }
}

// The logging macro
#ifdef DEBUG
#define LOG_INFO(msg) LogFunction("INFO", __FILE__, __LINE__, (msg))
#else
#define LOG_INFO(msg) ((void)0)
#endif

[[noreturn]] inline void log_error_impl(
    const char* file, int line, const std::string& msg
) {
  std::stringstream ss;
  ss << GetTimestamp()
     << " [ERROR] " << file << ":" << line << " " << msg;
  throw std::runtime_error(ss.str());
}

#define LOG_ERROR(msg) \
  do { log_error_impl(__FILE__, __LINE__, (msg)); } while (0)
