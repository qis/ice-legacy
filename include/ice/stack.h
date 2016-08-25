#pragma once
#include <functional>

namespace ice {
namespace stack {

using crash_handler = std::function<void(const char* trace)>;

void set_crash_handler(crash_handler handler);
auto get_crash_handler() -> crash_handler;

}  // namespace stack
}  // namespace ice
