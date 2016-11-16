#pragma once
#include <string>
#include <cstdint>

namespace ice {
namespace crypt {

std::string hash(const std::string& pass, unsigned long rounds = 12);
bool verify(const std::string& pass, const std::string& hash) noexcept;

}  // namespace crypt
}  // namespace ice
