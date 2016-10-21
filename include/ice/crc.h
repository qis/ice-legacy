#pragma once
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <cstdint>

namespace ice {

template <std::size_t S>
class crc_traits {};

template <>
class crc_traits<32> {
public:
  using type = typename std::uint32_t;
  static constexpr std::size_t size = 4;
};

template <>
class crc_traits<64> {
public:
  using type = typename std::uint64_t;
  static constexpr std::size_t size = 8;
};

template <std::size_t S>
class crc {
public:
  using value_type = typename crc_traits<S>::type;

  crc() = default;

  crc(std::string_view text)
  {
    append(text.data(), text.size());
  }

  crc(const void* data, std::size_t size)
  {
    append(data, size);
  }

  void append(const void* data, std::size_t size)
  {
    for (std::size_t i = 0; i < size; i++) {
      auto byte = static_cast<const std::uint8_t*>(data)[i];
      value_ = tab[static_cast<std::uint8_t>(value_) ^ byte] ^ (value_ >> 8);
    }
  }

  value_type value() const
  {
    return value_;
  }

  std::string str(bool uppercase = false) const
  {
    std::ostringstream oss;
    oss << std::setfill('0') << std::hex;
    if (uppercase) {
      oss << std::uppercase;
    }
    oss << std::setw(crc_traits<S>::size) << value();
    return oss.str();
  }

  void reset()
  {
    value_ = {};
  }

  static const value_type tab[256];

private:
  value_type value_ = 0;
};

template <std::size_t S>
std::ostream& operator<<(std::ostream& os, const crc<S>& hash)
{
  auto flags = os.flags();
  os << std::setfill('0') << std::hex << std::setw(crc_traits<S>::size) << hash.value();
  os.flags(flags);
  return os;
}

using crc32 = crc<32>;
using crc64 = crc<64>;

}  // namespace ice
