#pragma once
#include <ice/bitmask.h>
#include <chrono>
#include <filesystem>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <system_error>
#include <cstdint>

namespace ice {
namespace log {

using clock = std::chrono::system_clock;
using timestamp = clock::time_point;

enum class severity : std::uint8_t {
  emergency    = 0x0,
  alert        = 0x1,
  critical     = 0x2,
  error        = 0x3,
  warning      = 0x4,
  notice       = 0x5,
  info         = 0x6,
  debug        = 0x7,
};

enum class options : std::uint8_t {
  none         = 0x0,
  append       = 0x1,
  milliseconds = 0x2,
};

template<>
struct enable_bitmask_operators<options> {
  static constexpr bool value = true;
};

bool init(severity severity = severity::debug, options options = options::milliseconds) noexcept;
bool init(const std::filesystem::path& filename, severity severity = severity::debug, options options = options::append | options::milliseconds) noexcept;

class stream : public std::stringbuf, public std::ostream {
public:
  explicit stream(severity severity) :
    std::stringbuf(), std::ostream(this), severity_(severity)
  {}

  stream(stream&& other) = default;
  stream& operator=(stream&& other) = default;

  virtual ~stream();

private:
  timestamp timestamp_ = clock::now();
  severity severity_ = severity::info;
};

template<severity S>
class stream_proxy : public stream {
public:
  stream_proxy() : stream(S)
  {}

  stream_proxy(stream_proxy&& other) = default;
  stream_proxy& operator=(stream_proxy&& other) = default;

  virtual ~stream_proxy() final = default;
};

inline std::string format(std::string message)
{
  auto pos = message.find_last_not_of(" \t\n\v\f\r");
  if (pos != message.npos) {
    message.erase(pos + 1);
  }
  return message;
}

inline std::string format(const std::error_code& ec)
{
  std::ostringstream oss;
  oss << ec.category().name() << " (" << ec.value() << "): " << ec.message();
  return format(oss.str());
}

using emergency = stream_proxy<severity::emergency>;
using alert     = stream_proxy<severity::alert>;
using critical  = stream_proxy<severity::critical>;
using error     = stream_proxy<severity::error>;
using warning   = stream_proxy<severity::warning>;
using notice    = stream_proxy<severity::notice>;
using info      = stream_proxy<severity::info>;
using debug     = stream_proxy<severity::debug>;

}  // namespace log
}  // namespace ice
