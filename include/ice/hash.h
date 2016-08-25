#pragma once
#include <array>
#include <iomanip>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <cstdint>

namespace ice {
namespace hash {

template <typename Hash>
class crc {
public:
  using value_type = Hash;

  crc() = default;

  template <std::size_t N>
  crc(const char(&STR)[N])
  {
    append(STR, N - 1);
  }

  crc(const void* data, std::size_t size)
  {
    append(data, size);
  }

  crc(const std::string& text) : crc(text.data(), text.size())
  {}

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

  std::string str(bool uppercase = true) const
  {
    std::ostringstream oss;
    oss << std::setfill('0') << std::hex;
    if (uppercase) {
      oss << std::uppercase;
    }
    oss << std::setw(sizeof(value_type) * 2) << value();
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

template <typename Hash>
inline std::ostream& operator<<(std::ostream& os, const crc<Hash>& hash)
{
  return os << hash.str();
}

using crc32 = crc<std::uint32_t>;
using crc64 = crc<std::uint64_t>;

enum class sha_size : std::size_t {
  sha1   = 20,
  sha224 = 28,
  sha256 = 32,
  sha384 = 48,
  sha512 = 64,
};

template <sha_size Size>
class sha {
public:
  static const auto size = static_cast<std::size_t>(Size);
  using value_type = std::array<std::uint8_t, size>;

  sha();

  template <std::size_t N>
  sha(const char(&STR)[N]) : sha()
  {
    append(STR, N - 1);
  }

  sha(const void* data, std::size_t size) : sha()
  {
    append(data, size);
  }

  sha(const std::string& text) : sha(text.data(), text.size())
  {}

  ~sha();

  void append(const void* data, std::size_t size);

  value_type value();

  std::string str(bool uppercase = false) const
  {
    std::ostringstream oss;
    oss << std::setfill('0') << std::hex;
    if (uppercase) {
      oss << std::uppercase;
    }
    for (const auto c : value()) {
      oss << std::setw(2) << static_cast<std::size_t>(c);
    }
    return oss.str();
  }

  void reset();

private:
  struct context;
  std::unique_ptr<context> context_;
  std::optional<value_type> value_;
};

template <sha_size Size>
inline std::ostream& operator<<(std::ostream& os, const sha<Size>& hash)
{
  return os << hash.str();
}

using sha1   = sha<sha_size::sha1>;
using sha224 = sha<sha_size::sha224>;
using sha256 = sha<sha_size::sha256>;
using sha384 = sha<sha_size::sha384>;
using sha512 = sha<sha_size::sha512>;

}  // namespace hash
}  // namespace ice
