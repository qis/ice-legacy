#include <ice/log.h>
#include <ice/color.h>
#include <ice/exception.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <termios.h>
#include <algorithm>
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

std::ostream& format_time_point(std::ostream& os, const clock::time_point& time_point, bool milliseconds)
{
  auto time = clock::to_time_t(time_point);
  tm tm = {};
#ifndef _WIN32
  localtime_r(&time, &tm);
#else
  localtime_s(&tm, &time);
#endif
  auto flags = os.flags();
  os << std::setfill('0') << std::dec << std::setw(4) << (tm.tm_year + 1900) << '-' << std::setw(2) << (tm.tm_mon + 1)
     << '-' << std::setw(2) << tm.tm_mday << ' ' << std::setw(2) << tm.tm_hour << ':' << std::setw(2) << tm.tm_min
     << ':' << std::setw(2) << tm.tm_sec;
  if (milliseconds) {
    auto tsp = time_point.time_since_epoch();
    auto s = std::chrono::duration_cast<std::chrono::seconds>(tsp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tsp) - s;
    os << '.' << std::setw(3) << ms.count();
  }
  os.flags(flags);
  return os;
}

class sink {
public:
  virtual ~sink() = default;
  virtual void write(const std::vector<message>& messages) = 0;
};

class console : public sink {
public:
  explicit console(severity severity, options options) : severity_(severity), options_(options)
  {}

  virtual void write(const std::vector<message>& messages)
  {
    bool cout = false;
    bool cerr = false;
    for (const auto& message : messages) {
      auto severity = std::get<1>(message);
      if (severity > severity_) {
        continue;
      }
      std::ostream& os = severity < severity::warning ? std::cerr : std::cout;
      if (&os == &std::cout) {
        cout = true;
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
    if (cout) {
      std::cout << std::flush;
    }
    if (cerr) {
      std::cerr << std::flush;
    }
  }

  std::ostream& color(std::ostream& os)
  {
    return os << ice::color::reset;
  }

  std::ostream& color(std::ostream& os, severity severity)
  {
    switch (severity) {
    case severity::emergency: return os << ice::color::cyan;
    case severity::alert: return os << ice::color::blue;
    case severity::critical: return os << ice::color::magenta;
    case severity::error: return os << ice::color::red;
    case severity::warning: return os << ice::color::yellow;
    case severity::notice: return os << ice::color::green;
    case severity::info: return os << ice::color::reset;
    case severity::debug: return os << ice::color::grey;
    }
    return os;
  }

  std::ostream& format(std::ostream& os, const timestamp& timestamp)
  {
    return format_time_point(os, timestamp, ice::bitmask(options_ & options::milliseconds));
  }

  std::ostream& format(std::ostream& os, severity severity)
  {
    switch (severity) {
    case severity::emergency: return os << "emergency";
    case severity::alert: return os << "alert    ";
    case severity::critical: return os << "critical ";
    case severity::error: return os << "error    ";
    case severity::warning: return os << "warning  ";
    case severity::notice: return os << "notice   ";
    case severity::info: return os << "info     ";
    case severity::debug: return os << "debug    ";
    }
    return os << "unknown  ";
  }

protected:
  severity severity_;
  options options_;
};

class file : public console {
public:
  explicit file(const std::filesystem::path& filename, severity severity, options options)
    : console(severity, options), os_(filename, std::ios::binary | flags(options))
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

  static std::ios::openmode flags(options options)
  {
    return static_cast<std::uint8_t>(options) & static_cast<std::uint8_t>(options::append)
             ? std::ios::app
             : static_cast<std::ios::openmode>(0);
  }

private:
  std::ofstream os_;
};

class logger {
private:
  logger()
  {
    if (tcgetattr(0, &stdin_options_) == 0) {
      auto stdin_options = stdin_options_;
      stdin_options.c_lflag &= ~ECHOCTL;
      if (tcsetattr(0, TCSAFLUSH, &stdin_options) == 0) {
        stdin_options_set_ = true;
      }
    }
    thread_ = std::thread([this]() { run(); });
  }

public:
  ~logger()
  {
    if (stdin_options_set_) {
      tcsetattr(0, TCSAFLUSH, &stdin_options_);
    }
    stop();
  }

  void stop()
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

  struct termios stdin_options_ = {};
  bool stdin_options_set_ = false;
};

}  // namespace

bool init(severity severity, options options) noexcept
{
  try {
    try {
      logger::get().init(std::make_shared<console>(severity, options));
    }
    catch (const std::system_error& e) {
      std::cerr << "Could not initialize the logging facility.\n" << ice::log::format(e.code()) << std::endl;
      return false;
    }
    catch (const std::exception& e) {
      std::cerr << "Could not initialize the logging facility.\n" << ice::log::format(e.what()) << std::endl;
      return false;
    }
    catch (...) {
      std::cerr << "Could not initialize the logging facility.\n" << std::flush;
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
                << filename.u8string() << std::endl;
      return false;
    }
    catch (const std::exception& e) {
      std::cerr << "Could not initialize the logging facility.\n"
                << ice::log::format(e.what()) << '\n'
                << filename.u8string() << std::endl;
      return false;
    }
    catch (...) {
      std::cerr << "Could not initialize the logging facility.\n" << filename.u8string() << std::endl;
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

stream& operator<<(stream& s, const std::error_code& ec) noexcept
{
  try {
    auto& os = static_cast<std::ostream&>(s);
    os << ec.category().name() << ' ' << ec.value() << ": " << ec.message();
  }
  catch (...) {}
  return s;
}

stream& operator<<(stream& s, const std::exception_ptr& e) noexcept
{
  try {
    auto& os = static_cast<std::ostream&>(s);
    try {
      if (e) {
        std::rethrow_exception(e);
      }
    }
    catch (const ice::exception_stream<std::system_error>& e) {
      auto ec = e.code();
      os << ec.category().name() << ' ' << ec.value() << ": " << e.what();
      if (auto info = e.info()) {
        os << ": " << info;
      }
    }
    catch (const std::system_error& e) {
      auto ec = e.code();
      os << ec.category().name() << ' ' << ec.value() << ": " << e.what();
    }
    catch (const ice::exception& e) {
      os << e.what();
      if (auto info = e.info()) {
        os << ": " << info;
      }
    }
    catch (const std::exception& e) {
      os << e.what();
    }
    catch (...) {
      os << "unhandled exception";
    }
  }
  catch (...) {}
  return s;
}

stream& operator<<(stream& s, const std::chrono::milliseconds& duration)
{
  auto value = duration;
  auto negative = value < std::chrono::milliseconds(0);
  if (negative) {
    value = -value;
  }

  auto hours = std::chrono::duration_cast<std::chrono::hours>(value);
  auto minutes = std::chrono::duration_cast<std::chrono::minutes>(value - hours);
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(value - hours - minutes);
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(value - hours - minutes - seconds);

  auto& os = static_cast<std::ostream&>(s);
  auto flags = os.flags();
  os << std::setfill('0') << std::dec;
  if (negative) {
    os << '-';
  }
  os << std::setw(2) << hours.count() << ':';
  os << std::setw(2) << minutes.count() << ':';
  os << std::setw(2) << seconds.count() << '.';
  os << std::setw(3) << milliseconds.count();
  os.flags(flags);
  return s;
}

stream& operator<<(stream& s, const std::chrono::time_point<clock>& tp)
{
  format_time_point(static_cast<std::ostream&>(s), tp, true);
  return s;
}

}  // namespace log
}  // namespace ice
