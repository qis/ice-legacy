#pragma once
#include <cstdint>

namespace ice {

class base64 {
public:
  static std::uint8_t table(std::uint32_t index)
  {
    static const std::uint8_t data[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                        'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
    return data[index];
  }

  template <typename OctetIterator, typename OutputIterator>
  static OutputIterator encode(OctetIterator begin, OctetIterator end, OutputIterator out)
  {
    for (std::uint8_t octet[3]; begin != end;) {
      std::uint8_t n = 0;
      octet[n++] = *begin++;
      if (begin != end) {
        octet[n++] = *begin++;
      } else {
        octet[1] = 0x00;
      }
      if (begin != end) {
        octet[n++] = *begin++;
      } else {
        octet[2] = 0x00;
      }
      auto triple = static_cast<std::uint32_t>((octet[0] << 0x10) + (octet[1] << 0x08) + octet[2]);
      *out++ = table((triple >> 3 * 6) & 0x3F);
      *out++ = table((triple >> 2 * 6) & 0x3F);
      *out++ = table((triple >> 1 * 6) & 0x3F);
      *out++ = table((triple >> 0 * 6) & 0x3F);
      for (auto i = n; i < 3; i++) {
        *out++ = '=';
      }
    }
    return out;
  }
};

}  // namespace ice