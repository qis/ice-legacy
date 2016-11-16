#include <ice/sha.h>
#include <openssl/sha.h>

namespace ice {

// SHA1 =======================================================================

template <>
struct sha<1>::context : public SHA_CTX {};

template <>
sha<1>::sha() : context_(std::make_unique<context>()) {
  SHA1_Init(context_.get());
}

template <>
sha<1>::~sha() {}

template <>
void sha<1>::append(const void* data, std::size_t size) {
  SHA1_Update(context_.get(), data, size);
}

template <>
void sha<1>::finish() {
  SHA1_Final(reinterpret_cast<unsigned char*>(value_.data()), context_.get());
}

template <>
void sha<1>::reset() {
  value_ = {};
  SHA1_Init(context_.get());
}

// SHA224 =======================================================================

template <>
struct sha<224>::context : public SHA256_CTX {};

template <>
sha<224>::sha() : context_(std::make_unique<context>()) {
  SHA224_Init(context_.get());
}

template <>
sha<224>::~sha() {}

template <>
void sha<224>::append(const void* data, std::size_t size) {
  SHA224_Update(context_.get(), data, size);
}

template <>
void sha<224>::finish() {
  SHA224_Final(reinterpret_cast<unsigned char*>(value_.data()), context_.get());
}

template <>
void sha<224>::reset() {
  value_ = {};
  SHA224_Init(context_.get());
}

// SHA256 =======================================================================

template <>
struct sha<256>::context : public SHA256_CTX {};

template <>
sha<256>::sha() : context_(std::make_unique<context>()) {
  SHA256_Init(context_.get());
}

template <>
sha<256>::~sha() {}

template <>
void sha<256>::append(const void* data, std::size_t size) {
  SHA256_Update(context_.get(), data, size);
}

template <>
void sha<256>::finish() {
  SHA256_Final(reinterpret_cast<unsigned char*>(value_.data()), context_.get());
}

template <>
void sha<256>::reset() {
  value_ = {};
  SHA256_Init(context_.get());
}

// SHA384 =======================================================================

template <>
struct sha<384>::context : public SHA512_CTX {};

template <>
sha<384>::sha() : context_(std::make_unique<context>()) {
  SHA384_Init(context_.get());
}

template <>
sha<384>::~sha() {}

template <>
void sha<384>::append(const void* data, std::size_t size) {
  SHA384_Update(context_.get(), data, size);
}

template <>
void sha<384>::finish() {
  SHA384_Final(reinterpret_cast<unsigned char*>(value_.data()), context_.get());
}

template <>
void sha<384>::reset() {
  value_ = {};
  SHA384_Init(context_.get());
}

// SHA512 =======================================================================

template <>
struct sha<512>::context : public SHA512_CTX {};

template <>
sha<512>::sha() : context_(std::make_unique<context>()) {
  SHA512_Init(context_.get());
}

template <>
sha<512>::~sha() {}

template <>
void sha<512>::append(const void* data, std::size_t size) {
  SHA512_Update(context_.get(), data, size);
}

template <>
void sha<512>::finish() {
  SHA512_Final(reinterpret_cast<unsigned char*>(value_.data()), context_.get());
}

template <>
void sha<512>::reset() {
  value_ = {};
  SHA512_Init(context_.get());
}

}  // namespace ice
