#pragma once
#include <array>
#include <memory>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <cstdint>

namespace ice {

template <std::size_t S>
class sha_traits {};

template <>
class sha_traits<1> {
public:
  static constexpr std::size_t size = 20;
};

template <>
class sha_traits<224> {
public:
  static constexpr std::size_t size = 28;
};

template <>
class sha_traits<256> {
public:
  static constexpr std::size_t size = 32;
};

template <>
class sha_traits<384> {
public:
  static constexpr std::size_t size = 48;
};

template <>
class sha_traits<512> {
public:
  static constexpr std::size_t size = 64;
};

template <std::size_t S>
class sha {
public:
  using value_type = typename std::array<std::uint8_t, sha_traits<S>::size>;

  sha();

  sha(std::string_view text) : sha()
  {
    append(text.data(), text.size());
    finish();
  }

  sha(const void* data, std::size_t size) : sha()
  {
    append(data, size);
    finish();
  }

  ~sha();

  void append(const void* data, std::size_t size);
  void finish();

  value_type& value()
  {
    return value_;
  }

  const value_type& value() const
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
    for (auto byte : value_) {
      oss << std::setw(2) << static_cast<unsigned>(byte);
    }
    return oss.str();
  }

  void reset();

private:
  struct context;
  std::unique_ptr<context> context_;
  value_type value_ = {};
};

template <std::size_t S>
std::ostream& operator<<(std::ostream& os, const sha<S>& hash)
{
  auto flags = os.flags();
  os << std::setfill('0') << std::hex;
  for (auto byte : hash.value()) {
    os << std::setw(2) << static_cast<unsigned>(byte);
  }
  os.flags(flags);
  return os;
}

using sha1 = sha<1>;
using sha224 = sha<224>;
using sha256 = sha<256>;
using sha384 = sha<384>;
using sha512 = sha<512>;

}  // namespace ice
