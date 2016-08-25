#pragma once
#include <array>
#include <ostream>
#include <string>
#include <cstdint>

namespace ice {

struct uuid {
  using data_type = std::array<std::uint8_t, 16>;

  using u32 = std::uint32_t;
  using u16 = std::uint16_t;
  using u8  = std::uint8_t;

  u32 tl;    // time-low
  u16 tm;    // time-mid
  u16 thv;   // time-high-and-version
  u8  csr;   // clock-seq-and-reserved
  u8  csl;   // clock-seq-low
  u8  n[6];  // node

  constexpr uuid() :
    tl(0), tm(0), thv(0), csr(0), csl(0), n{ 0, 0, 0, 0, 0, 0 }
  {}

  constexpr uuid(u32 tl, u16 tm, u16 thv, u8 csr, u8 csl, u8 n[6]) :
    tl(tl), tm(tm), thv(thv), csr(csr), csl(csl), n{ n[0], n[1], n[2], n[3], n[4], n[5] }
  {}

  constexpr uuid(u32 tl, u16 tm, u16 thv, u8 csr, u8 csl, u8 n0, u8 n1, u8 n2, u8 n3, u8 n4, u8 n5) :
    tl(tl), tm(tm), thv(thv), csr(csr), csl(csl), n{ n0, n1, n2, n3, n4, n5 }
  {}

  // Parses a string into the UUID.
  explicit uuid(const std::string& str);

  // Parses a portable binary blob into the UUID.
  explicit uuid(const data_type& data);

  // Parses a portable binary blob into the UUID.
  explicit uuid(const std::uint8_t* data, std::size_t size);

  // Formats the UUID as a string.
  std::string str() const;

  // Formats the UUID as a portable binary blob.
  data_type data() const;

  // Generates a random UUID (version 4).
  static uuid generate();

  // Checks if the given string is a UUID.
  static bool check(const std::string& str);
};

inline constexpr bool operator==(const ice::uuid& a, const ice::uuid& b)
{
  return
    a.tl == b.tl && a.tm == b.tm && a.thv == b.thv && a.csr == b.csr && a.csl == b.csl &&
    a.n[0] == b.n[0] && a.n[1] == b.n[1] && a.n[2] == b.n[2] &&
    a.n[3] == b.n[3] && a.n[4] == b.n[4] && a.n[5] == b.n[5];
}

inline constexpr bool  operator!=(const ice::uuid& a, const ice::uuid& b)
{
  return !operator==(a, b);
}

inline std::ostream& operator<<(std::ostream& os, const ice::uuid& uuid)
{
  return os << uuid.str();
}

}  // namespace ice
