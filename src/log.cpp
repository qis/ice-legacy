#include <ice/log.h>
#include <ice/exception.h>
#include <ice/stack.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <condition_variable>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

namespace ice {
namespace log {
namespace {

using message = std::tuple<timestamp, severity, std::string>;

class sink {
public:
  virtual ~sink() = default;
  virtual void write(const std::vector<message>& messages) = 0;
};

class console : public sink {
public:
  explicit console(severity severity, options options) :
    severity_(severity), options_(options)
  {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO console_info = {};
    clog_ = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(clog_, &console_info);
    clog_attr_ = console_info.wAttributes;
    cerr_ = GetStdHandle(STD_ERROR_HANDLE);
    GetConsoleScreenBufferInfo(cerr_, &console_info);
    cerr_attr_ = console_info.wAttributes;
#endif
  }

  virtual void write(const std::vector<message>& messages)
  {
    bool clog = false;
    bool cerr = false;
    for (const auto& message : messages) {
      auto severity = std::get<1>(message);
      if (severity > severity_) {
        continue;
      }
      std::ostream& os = severity < severity::warning ? std::cerr : std::clog;
      if (&os == &std::clog) {
        clog = true;
      } else {
        cerr = true;
      }
      format(os, std::get<0>(message));
      os << " [";
      if (severity != severity::info) {
        color(os, severity);
      }
      format(os, severity);
      if (severity != severity::info) {
        color(os);
      }
      os << "] ";
      if (severity > severity::info) {
        color(os, severity::debug);
      }
      os << std::get<2>(message);
      if (severity > severity::info) {
        color(os);
      }
      os << '\n';
    }
    if (clog) {
      std::clog << std::flush;
    }
    if (cerr) {
      std::cerr << std::flush;
    }
  }

  std::ostream& color(std::ostream& os)
  {
#ifdef _WIN32
    if (&os == &std::clog) {
      SetConsoleTextAttribute(clog_, clog_attr_);
    } else {
      SetConsoleTextAttribute(cerr_, cerr_attr_);
    }
    return os;
#else
    return os << "\x1b[0m";
#endif
  }

  std::ostream& color(std::ostream& os, severity severity)
  {
#ifdef _WIN32
    auto handle = (&os == &std::clog ? clog_ : cerr_);
    switch (severity) {
    case severity::emergency: SetConsoleTextAttribute(handle, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY); return os;
    case severity::alert:     SetConsoleTextAttribute(handle, FOREGROUND_BLUE | FOREGROUND_INTENSITY); return os;
    case severity::critical:  SetConsoleTextAttribute(handle, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY); return os;
    case severity::error:     SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_INTENSITY); return os;
    case severity::warning:   SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); return os;
    case severity::notice:    SetConsoleTextAttribute(handle, FOREGROUND_GREEN | FOREGROUND_INTENSITY); return os;
    case severity::info:      SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); return os;
    case severity::debug:     SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY); return os;
    }
#else
    switch (severity) {
    case severity::emergency: return os << "\x1b[0;36m";
    case severity::alert:     return os << "\x1b[0;34m";
    case severity::critical:  return os << "\x1b[0;35m";
    case severity::error:     return os << "\x1b[0;31m";
    case severity::warning:   return os << "\x1b[0;33m";
    case severity::notice:    return os << "\x1b[0;32m";
    case severity::info:      return os << "\x1b[0m";
    case severity::debug:     return os << "\x1b[1;30m";
    }
#endif
    return color(os);
  }

  std::ostream& format(std::ostream& os, timestamp timestamp)
  {
    auto time = std::chrono::system_clock::to_time_t(timestamp);
    tm tm = {};
  #ifndef _WIN32
    localtime_r(&time, &tm);
  #else
    localtime_s(&tm, &time);
  #endif
    os << std::setfill('0')
       << std::setw(4) << (tm.tm_year + 1900) << '-'
       << std::setw(2) << (tm.tm_mon + 1) << '-'
       << std::setw(2) << tm.tm_mday << ' '
       << std::setw(2) << tm.tm_hour << ':'
       << std::setw(2) << tm.tm_min << ':'
       << std::setw(2) << tm.tm_sec;
    if (ice::bitmask(options_ & options::milliseconds)) {
      auto tsp = timestamp.time_since_epoch();
      auto s = std::chrono::duration_cast<std::chrono::seconds>(tsp).count();
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tsp).count() - (s * 1000);
      os << '.' << std::setw(3) << ms;
    }
    return os;
  }

  std::ostream& format(std::ostream& os, severity severity)
  {
    switch (severity) {
    case severity::emergency: return os << "emergency";
    case severity::alert:     return os << "alert    ";
    case severity::critical:  return os << "critical ";
    case severity::error:     return os << "error    ";
    case severity::warning:   return os << "warning  ";
    case severity::notice:    return os << "notice   ";
    case severity::info:      return os << "info     ";
    case severity::debug:     return os << "debug    ";
    }
    return os << "unknown  ";
  }

protected:
  severity severity_;
  options options_;
#ifdef _WIN32
  DWORD mode_ = 0;
  HANDLE clog_ = nullptr;
  WORD clog_attr_ = 0;
  HANDLE cerr_ = nullptr;
  WORD cerr_attr_ = 0;
#else
  termios tp_{};
#endif
};

class file : public console {
public:
  explicit file(const std::filesystem::path& filename, severity severity, options options) :
    console(severity, options), os_(filename, std::ios::binary | flags(options))
  {
    if (!os_) {
      throw std::system_error(errno, std::system_category());
    }
  }

  virtual void write(const std::vector<message>& messages) final
  {
    console::write(messages);
    for (const auto& message : messages) {
      auto severity = std::get<1>(message);
      if (severity > severity_) {
        continue;
      }
      format(os_, std::get<0>(message));
      os_ << " [";
      format(os_, severity);
      os_ << "] " << std::get<2>(message) << '\n';
    }
    os_ << std::flush;
  }

  static int flags(options options)
  {
    return static_cast<std::uint8_t>(options) & static_cast<std::uint8_t>(options::append) ? std::ios::app : 0;
  }

private:
  std::ofstream os_;
};

class logger {
private:
  logger()
  {
    if (!ice::stack::get_crash_handler()) {
      ice::stack::set_crash_handler([](const char* trace) {
        try {
          ice::log::critical() << "Unhandled exception.\n"
            << trace ? trace : "";
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          std::cin.get();
        }
        catch (...) {
        }
      });
    }
    thread_ = std::thread([this]() {
      run();
    });
  }

public:
  ~logger()
  {
    stop_ = true;
    cv_.notify_one();
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  void init(std::shared_ptr<sink> sink)
  {
    sink_ = std::move(sink);
  }

  void write(timestamp timestamp, severity severity, std::string text)
  {
    if (sink_) {
      std::lock_guard<std::mutex> lock(mutex_);
      messages_.emplace_back(timestamp, severity, std::move(text));
      cv_.notify_one();
    }
  }

  static logger& get()
  {
    static logger logger;
    return logger;
  }

private:
  void run()
  {
    while (!stop_) {
      std::vector<message> messages;
      {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!stop_ && messages_.empty()) {
          cv_.wait(lock);
        }
        messages = std::move(messages_);
        messages_.clear();
      }
      if (!messages.empty()) {
        if (auto sink = sink_) {
          sink->write(messages);
        }
      }
    }
  }

  bool stop_ = false;
  std::thread thread_;
  std::condition_variable cv_;
  std::shared_ptr<sink> sink_;
  std::vector<message> messages_;
  std::mutex mutex_;
};

}  // namespace

bool init(severity severity, options options) noexcept
{
  try {
    try {
      logger::get().init(std::make_shared<console>(severity, options));
    }
    catch (const std::system_error& e) {
      std::cerr << "Could not initialize the logging facility.\n"
        << ice::log::format(e.code()) << std::endl;
      return false;
    }
    catch (const std::exception& e) {
      std::cerr << "Could not initialize the logging facility.\n"
        << ice::log::format(e.what()) << std::endl;
      return false;
    }
    catch (...) {
      std::cerr << "Could not initialize the logging facility.\n"
        << std::flush;
      return false;
    }
  }
  catch (...) {
    return false;
  }
  return true;
}

bool init(const std::filesystem::path& filename, severity severity, options options) noexcept
{
  try {
    try {
      logger::get().init(std::make_shared<file>(filename, severity, options));
    }
    catch (const std::system_error& e) {
      std::cerr << "Could not initialize the logging facility.\n"
        << ice::log::format(e.code()) << '\n'
        << std::filesystem::canonical(filename).u8string() << std::endl;
      return false;
    }
    catch (const std::exception& e) {
      std::cerr << "Could not initialize the logging facility.\n"
        << ice::log::format(e.what()) << '\n'
        << std::filesystem::canonical(filename).u8string() << std::endl;
      return false;
    }
    catch (...) {
      std::cerr << "Could not initialize the logging facility.\n"
        << std::filesystem::canonical(filename).u8string() << std::endl;
      return false;
    }
  }
  catch (...) {
    return false;
  }
  return true;
}

stream::~stream()
{
  auto s = str();
  auto pos = s.find_last_not_of(" \t\n\v\f\r");
  if (pos != s.npos) {
    s.erase(pos + 1);
  }
  s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
  logger::get().write(timestamp_, severity_, std::move(s));
}

}  // namespace log
}  // namespace ice
