#include <ice/crypt.h>
#include <crypt/blowfish.h>
#include <openssl/rand.h>
#include <array>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ice {
namespace crypt {
namespace {

constexpr int part_info_size = 7;
constexpr int part_salt_size = 22;
constexpr int part_hash_size = 31;

constexpr int salt_size = part_info_size + part_salt_size;
constexpr int hash_size = part_info_size + part_salt_size + part_hash_size;

}  // namespace

std::string hash(const std::string& pass, unsigned long rounds) {
  // Generate random bytes.
  std::array<char, salt_size + 1> rand;
  RAND_bytes(reinterpret_cast<unsigned char*>(&rand[0]), salt_size + 1);

  // Generate salt.
  std::array<char, salt_size + 1> salt;
  if (!crypt_salt("$2b$", rounds, rand.data(), salt_size + 1, &salt[0], salt_size + 1)) {
    throw std::runtime_error("crypt salt error");
  }

  // Generate hash.
  std::string hash;
  hash.resize(hash_size + 1);
  if (!crypt_hash(pass.data(), salt.data(), &hash[0], hash_size + 1)) {
    throw std::runtime_error("crypt hash error");
  }
  hash.resize(hash_size);
  return hash;
}

bool verify(const std::string& pass, const std::string& hash) noexcept {
  // Generate hash.
  std::array<char, hash_size + 1> test;
  if (!crypt_hash(pass.data(), hash.data(), &test[0], hash_size + 1)) {
    return false;
  }

  // Compare supplied hash to the generated hash.
  return hash == std::string_view(test.data(), hash_size);
}

}  // namespace crypt
}  // namespace ice
