#include <ice/uuid.h>
#include <ice/exception.h>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <random>
#include <cstdio>

// Number of bytes in a formatted UUID string without the terminating character.
#define UUID_FORMAT_SIZE (8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12)

// Format for sscanf and snprintf.
#define UUID_FORMAT "%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx"

// Number of elements that have to be parsed/formatted by sscanf and snprintf.
#define UUID_FORMAT_COUNT 11

namespace ice {

uuid::uuid(std::string_view str)
{
  auto count = std::sscanf(str.data(), UUID_FORMAT, &data.v.tl, &data.v.tm, &data.v.thv, &data.v.csr, &data.v.csl,
                           &data.v.n[0], &data.v.n[1], &data.v.n[2], &data.v.n[3], &data.v.n[4], &data.v.n[5]);
  if (count != UUID_FORMAT_COUNT) {
    throw ice::runtime_error("uuid format error") << str;
  }
}

std::string uuid::str() const
{
  std::string str;
  str.resize(UUID_FORMAT_SIZE + 1);
  auto size = std::snprintf(&str[0], UUID_FORMAT_SIZE + 1, UUID_FORMAT, data.v.tl, data.v.tm, data.v.thv, data.v.csr,
                            data.v.csl, data.v.n[0], data.v.n[1], data.v.n[2], data.v.n[3], data.v.n[4], data.v.n[5]);
  if (size != UUID_FORMAT_SIZE) {
    throw ice::runtime_error("uuid format size error");
  }
  str.resize(static_cast<std::size_t>(size));
  return str;
}

uuid uuid::generate()
{
  static thread_local std::random_device rd;
  static thread_local std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<std::uint64_t>::max());

  ice::uuid uuid;

  // Set the UUID to a random value.
  uuid.data.s[0] = dist(rd);
  uuid.data.s[1] = dist(rd);

  // Set the UUID version according to RFC-4122 (Section 4.2).
  uuid.data.v.thv = static_cast<decltype(uuid.data.v.thv)>((uuid.data.v.thv & 0x0FFF) | 0x4000);
  uuid.data.v.csr = static_cast<decltype(uuid.data.v.csr)>((uuid.data.v.csr & 0x3F) | 0x80);

  return uuid;
}

bool uuid::check(std::string_view str)
{
  if (str.size() != 36) {
    return false;
  }
  for (std::size_t i = 0, size = str.size(); i < size; i++) {
    auto c = str[i];
    switch (i) {
    case 8:
    case 13:
    case 18:
    case 23:
      if (c != '-') {
        return false;
      }
      break;
    default:
      if ((c < '0' || c > '9') && (c < 'a' || c > 'f')) {
        return false;
      }
      break;
    }
  }
  return true;
}

}  // namespace ice
