#pragma once
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>
#include <cstdint>

namespace ice {
namespace detail {

template <typename T>
inline std::enable_if_t<std::is_integral<T>::value, std::string> hex(T value, bool uppercase = true)
{
  std::ostringstream oss;
  oss << std::hex << std::setfill('0');
  if (uppercase) {
    oss << std::uppercase;
  }
  oss << std::setw(sizeof(value) * 2) << static_cast<std::uint64_t>(value);
  return oss.str();
}

template <typename T>
inline std::enable_if_t<!std::is_integral<T>::value, std::string> hex(const T& range, bool uppercase = true)
{
  std::ostringstream oss;
  oss << std::hex << std::setfill('0');
  if (uppercase) {
    oss << std::uppercase;
  }
  for (auto e : range) {
    oss << std::setw(sizeof(e) * 2) << static_cast<std::uint64_t>(e);
  }
  return oss.str();
}

}  // namespace detail

template <typename T>
inline std::string hex(T&& range, bool uppercase = true)
{
  return detail::hex(std::forward<T>(range), uppercase);
}

template <typename T>
inline std::string hex(std::initializer_list<T> list, bool uppercase = true)
{
  return detail::hex(list, uppercase);
}

}  // namespace ice