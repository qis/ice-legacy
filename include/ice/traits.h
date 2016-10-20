#pragma once

namespace ice {

template <typename Json, typename T>
struct type_traits {
  static constexpr bool enable = false;
};

}  // namespace ice