# ICE
C++17 framework for Visual Studio 2015, Visual Studio 2015 Clang/C2 and Clang 4.0.

## ice::application
Retrieves information about the application executable.

```cpp
#include <ice/application.h>
#include <iostream>

int main() {
  const auto path = ice::application::path();
  std::cout << path.filename().u8string() << std::endl;
  std::cout << path.parent_path().u8string() << std::endl;
  std::cout << path.u8string() << std::endl;
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

namespace test {

enum class options {
  none = 0x0,
  one  = 0x1,
  two  = 0x2,
};

}  // namespace test

template<>
struct enable_bitmask_operators<test::options> {
  static constexpr bool value = true;
};

int main() {
  auto opt = test::options::one | test::options::two;
  assert(ice::bitmask(opt & test::options::one));
}
```

See <https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html> for more information.

## ice::color
Stream operators that add color support for `std::cout`, `std::cerr` and `std::clog`.

```cpp
#include <ice/color.h>
#include <iostream>

int main() {
  std::cout << ice::color::red << ice::color::on_yellow << "red on yellow"
    << ice::color::reset << " test" << std::endl;
}
```

See <http://termcolor.readthedocs.io/> for more information.

## ice::crypt
Openwall bcrypt wrapper.

```cpp
#include <ice/crypt.h>
#include <iostream>

int main() {
  auto hash = ice::crypt::hash("test");
  std::cout << ice::crypt::verify("test", hash) << std::endl;  // 1
  std::cout << ice::crypt::verify("Test", hash) << std::endl;  // 0
}
```

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

## ice::crc
CRC hashing functions.

```cpp
#include <ice/crc.h>
#include <iostream>

int main() {
  // Use the simple interface to print the CRC32 hash for the given text.
  std::cout << ice::crc32("test") << std::endl;

  // Calculate the CRC64 hash.
  ice::crc64 hash("test");

  // Get the hash value.
  std::uint64_t value = hash.value();

  // Get the hash value string.
  std::string string = hash.str();  // `hash.str(true)` for uppercase

  // Reset the hash object so that it can be reused.
  //hash.reset();
}
```

## ice::sha
SHA hashing functions.

```cpp
#include <ice/sha.h>
#include <iostream>

int main() {
  // Use the simple interface to print the SHA1 hash for the given text.
  std::cout << ice::sha1("test") << std::endl;

  // Create a reusable SHA512 object.
  ice::sha512 hash;

  // Calculate the SHA512 hash.
  hash.append("te", 2);
  hash.append("st", 2);
  hash.finish();

  // Process the hash value.
  std::cout << std::setfill('0') << std::hex;
  for (auto byte : hash.value()) {
    std::cout << std::setw(2) << static_cast<unsigned>(byte);
  }
  std::cout << std::endl;

  // Get the hash value string.
  std::string string = hash.str();  // `hash.str(true)` for uppercase

  // Reset the hash object so that it can be reused.
  //hash.reset();
}
```

## ice::json
JSON for Modern C++.

```cpp
#include <ice/json.h>
#include <optional>
#include <iostream>

struct opt {
  bool valid = false;
};

namespace ice {
template <>
struct json_type<json, opt> {
  static constexpr bool enable = true;
  static json set(const opt& value) {
    return { { "valid", value.valid } };
  }
  static opt get(const json& value) {
    return { value["valid"].get<bool>() };
  }
};
}  // namespace ice

int main() {
  ice::json value;
  value["one"] = 1;
  value["two"] = std::optional<int>();
  value["opt"] = opt{ true };
  std::cout << value["opt"].get<opt>().valid << '\n' << value << std::endl;
}
```

See <https://github.com/nlohmann/json> for more information.

## ice::log
Asynchronous logging facility. Initializing this library sets a simple crash handler
using the `ice::stack` APIs.

```cpp
#include <ice/log.h>
#include <stdexcept>

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

  try {
    throw std::runtime_error("test");
  }
  catch (...) {
    ice::log::error() << "error: " << std::current_exception();
  }
  return 0;
}
```

```
2016-08-25 12:29:54.079 [info     ] Hello World #3
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
