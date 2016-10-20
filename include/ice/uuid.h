#pragma once
#include <ice/traits.h>
#include <ostream>
#include <string>
#include <cstdint>

namespace ice {

struct uuid {
  union {
    struct {
      std::uint32_t tl;   // time-low
      std::uint16_t tm;   // time-mid
      std::uint16_t thv;  // time-high-and-version
      std::uint8_t csr;   // clock-seq-and-reserved
      std::uint8_t csl;   // clock-seq-low
      std::uint8_t n[6];  // node
    } v;
    std::uint64_t s[2];
  } data;

  constexpr uuid() : data({})
  {}

  // Parses a string.
  explicit uuid(const std::string& str);

  // Formats the UUID.
  std::string str() const;

  // Generates a random UUID (version 4).
  static uuid generate();

  // Checks if the given string is a UUID.
  static bool check(const std::string& str);
};

inline constexpr bool operator==(const ice::uuid& a, const ice::uuid& b)
{
  return a.data.s[0] == b.data.s[0] && a.data.s[1] == b.data.s[1];
}

inline constexpr bool operator!=(const ice::uuid& a, const ice::uuid& b)
{
  return !operator==(a, b);
}

inline constexpr bool operator<(const ice::uuid& a, const ice::uuid& b)
{
  return a.data.s[0] < b.data.s[0] || (a.data.s[0] == b.data.s[0] && a.data.s[1] < b.data.s[1]);
}

inline constexpr bool operator<=(const ice::uuid& a, const ice::uuid& b)
{
  return a == b || a < b;
}

inline constexpr bool operator>(const ice::uuid& a, const ice::uuid& b)
{
  return a.data.s[0] > b.data.s[0] || (a.data.s[0] == b.data.s[0] && a.data.s[1] > b.data.s[1]);
}

inline constexpr bool operator>=(const ice::uuid& a, const ice::uuid& b)
{
  return a == b || a > b;
}

inline std::ostream& operator<<(std::ostream& os, const ice::uuid& uuid)
{
  return os << uuid.str();
}

template <typename Json>
struct type_traits<Json, uuid> {
  static constexpr bool enable = true;

  static Json set(const uuid& value)
  {
    return value.str();
  }

  static uuid get(const Json& json)
  {
    auto str = json.template get<std::string>();
    return uuid(str);
  }
};

}  // namespace ice
