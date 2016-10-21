#pragma once

namespace ice {

template <typename Json, typename T>
struct json_type {
  static constexpr bool enable = false;
};

}  // namespace ice
