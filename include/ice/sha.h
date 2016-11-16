#pragma once
#include <gsl/span>
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
  sha();

  sha(std::string_view text) : sha() {
    append(text.data(), text.size());
    finish();
  }

  sha(gsl::span<const gsl::byte> blob) : sha() {
    append(blob.data(), blob.size());
    finish();
  }

  ~sha();

  void append(const void* data, std::size_t size);
  void finish();

  gsl::span<const gsl::byte> value() const {
    return gsl::span<const gsl::byte>(value_.data(), value_.size());
  }

  std::string str(bool uppercase = false) const {
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
  std::array<gsl::byte, sha_traits<S>::size> value_ = {};
};

template <std::size_t S>
std::ostream& operator<<(std::ostream& os, const sha<S>& hash) {
  return os << hash.str();
}

using sha1 = sha<1>;
using sha224 = sha<224>;
using sha256 = sha<256>;
using sha384 = sha<384>;
using sha512 = sha<512>;

}  // namespace ice
