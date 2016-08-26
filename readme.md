# ICE
A small C++17 framework.

## ice::app
Application specific functions.

```cpp
#include <ice/app.h>

int main() {
  // Get an absolute path to the application executable file.
  std::filesystem::path file = ice::app::file();

  // Get an absolute path to the application executable directory.
  std::filesystem::path path = ice::app::path();
}
```

## ice::base
Encodes data as [base64](https://en.wikipedia.org/wiki/Base64).

```cpp
#include <ice/base.h>
#include <iterator>
#include <string>

int main() {
  // Encode a string as base64.
  std::string src = "test";
  std::string dst;
  ice::base64::encode(src.begin(), src.end(), std::back_insert_iterator<std::string>(dst));
}
```

## ice::bitmask
Utility header that helps using enum classes as bitfields.

```cpp
#include <ice/bitmask.h>
#include <assert.h>

enum class options {
  none = 0x0,
  one  = 0x1,
  two  = 0x2,
};

template<>
struct enable_bitmask_operators<options> {
  static constexpr bool value = true;
};

int main() {
  auto opt = options::one | options::two;
  assert(ice::bitmask(opt & options::one));
}
```

See <https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html> for more information.

## ice::color
Stream operators that add color support for `std::cout`, `std::cerr` and `std::clog`.

```cpp
#include <ice/color.h>

int main() {
  std::cout << ice::color::red << ice::color::on_yellow << "red on yellow"
    << ice::color::reset << " test" << std::endl;
}
```

See <http://termcolor.readthedocs.io/> for more information.

## ice::date
Date and time library for use with C++11 and C++14.

```cpp
#include <ice/date.h>
#include <iostream>

int main() {
  using namespace ice::date::literals;
  std::cout << ice::date::weekday{ 2016_y / 4 / 1 } << std::endl;
}
```

See <https://howardhinnant.github.io/date/date.html> for more information.

## ice::exception
Exception wrapper that acts as a stream.

```cpp
#include <ice/exception.h>
#include <iostream>

int main() {
  try {
    throw ice::runtime_error("error") << "additional information";
  }
  catch (const ice::exception& e) {
    std::cerr << e.what() << '\n' << (e.info() ? e.info() : "") << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
```

## ice::giant
Handles little/big endianness.

```cpp
#include <ice/giant.h>
#include <iomanip>
#include <iostream>
#include <cstdint>

int main() {
  auto u32 = 0x12345678UL;
  auto u32le = ice::giant::htobe(u32);
  std::cout << std::setfill('0') << std::hex
    << "0x" << std::setw(sizeof(u32le) * 2) << u32le << std::endl;
}
```

See <https://github.com/r-lyeh/giant> for more information.

## ice::hash
Hashing functions.

```cpp
#include <ice/hash.h>
#include <iostream>

int main() {
  // Use the simple interface to print the SHA1 hash for the given text.
  std::cout << ice::hash::sha1("test") << std::endl;

  {
    // Calculate the CRC32 hash.
    ice::hash::crc32 hash("test");

    // Get the hash value.
    std::uint32_t value = hash.value();

    // Get the hash value string.
    std::string string = hash.str();  // `hash.str(false)` for lowercase
  }

  {
    // Calculate the CRC64 hash.
    ice::hash::crc64 hash("test");

    // Get the hash value.
    std::uint64_t value = hash.value();

    // Get the hash value string.
    std::string string = hash.str();  // `hash.str(false)` for lowercase
  }

  {
    // Create a reusable SHA512 object.
    ice::hash::sha512 hash;

    // Calculate the SHA512 hash.
    hash.append("te", 2);
    hash.append("st", 2);

    // Get the hash value.
    std::array<std::uint8_t, ice::hash::sha512::size> value = hash.value();

    // Get the hash value string.
    std::string string = hash.str();  // `hash.str(true)` for uppercase

    // Reset the SHA1 object so that it can be reused.
    hash.reset();
  }
}
```

## ice::json
JSON for Modern C++.

```cpp
#include <ice/json.h>
#include <iostream>

int main() {
  ice::json value;
  value["one"] = 1;
  std::cout << value["one"].get<int>() << '\n' << value << std::endl;
}
```

See <https://github.com/nlohmann/json> for more information.

## ice::log
Asynchronous logging facility. Initializing this library sets a simple crash handler
using the `ice::stack` APIs.

```cpp
#include <ice/log.h>

#define LOG_FILENAME "app.log"

int main() {
#ifndef LOG_FILENAME
  // Log to console.
  ice::log::init();
#else
  // Log to console and file.
  if (!ice::log::init(LOG_FILENAME)) {
    return 1;
  }
#endif
  ice::log::info() << "Hello World #" << 3;
  return 0;
}
```

```
2016-08-25 12:29:54.079 [info     ] Hello World #3
```

## ice::stack
Portable crash handler.

```cpp
#include <ice/stack.h>
#include <iostream>
#include <stdexcept>

int main() {
  ice::stack::set_crash_handler([](const char* trace) {
    std::cerr << trace << std::endl;
  });
  throw std::runtime_error("test");
}
```

```
RaiseException + 0x68
app: _CxxThrowException + 0x130
app: main + 0x79
app: invoke_main + 0x34
app: __scrt_common_main_seh + 0x127
app: __scrt_common_main + 0xe
app: mainCRTStartup + 0x9
BaseThreadInitThunk + 0x22
RtlUserThreadStart + 0x34
```

## ice::uuid
Class for [UUID](https://en.wikipedia.org/wiki/Universally_unique_identifier) handling.

```cpp
#include <ice/uuid.h>
#include <iostream>

int main() {
  // Create a zero inizialized UUID.
  ice::uuid null;

  // Generate a random UUID (version 4).
  ice::uuid uuid = ice::uuid::generate();

  // Get the raw UUID data.
  std::array<std::uint8_t, 16> data = uuid.data();

  // Get the UUID string.
  std::string str = uuid.str();

  // Check if the string is a valid UUID.
  bool valid = ice::uuid::check(str);

  // Parse the string as an UUID.
  ice::uuid test(str);

  // Print the UUID.
  std::cout << test << std::endl;
}
```
