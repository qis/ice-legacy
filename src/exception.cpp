#include <ice/exception.h>
#include <algorithm>

namespace ice {
namespace {

std::string strip_whitespace(std::string s) {
  auto pos = s.find_last_not_of(" \t\n\v\f\r");
  if (pos != s.npos) {
    s.erase(pos + 1);
  }
  s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
  return s;
}

}  // namespace

std::string format(const std::error_code& ec) {
  std::ostringstream oss;
  oss << ec.category().name() << ' ' << ec.value() << ": " << ec.message();
  return strip_whitespace(oss.str());
}

std::string format(const std::exception_ptr& ep) {
  std::ostringstream oss;
  try {
    if (ep) {
      std::rethrow_exception(ep);
    }
  }
  catch (const ice::exception& e) {
    if (auto se = dynamic_cast<const std::system_error*>(&e)) {
      auto ec = se->code();
      oss << ec.category().name() << ' ' << ec.value() << ": ";
    }
    oss << e.what();
    if (auto info = e.info()) {
      oss << ": " << info;
    }
  }
  catch (const std::exception& e) {
    if (auto se = dynamic_cast<const std::system_error*>(&e)) {
      auto ec = se->code();
      oss << ec.category().name() << ' ' << ec.value() << ": ";
    }
    oss << e.what();
  }
  catch (...) {
    oss << "unhandled exception";
  }
  return strip_whitespace(oss.str());
}

}  // namespace ice
