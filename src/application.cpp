#include <ice/application.h>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <limits.h>
#include <stdlib.h>
#endif
#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

namespace ice {
namespace application {

// http://www.tech.theplayhub.com/finding_current_executables_path_without_procselfexe-7
// =====================================================================================
// Mac OS X: _NSGetExecutablePath() (man 3 dyld)
// Linux: readlink /proc/self/exe
// Solaris: getexecname()
// FreeBSD: sysctl CTL_KERN KERN_PROC KERN_PROC_PATHNAME -1
// FreeBSD if it has procfs: readlink /proc/curproc/file (FreeBSD doesn't have procfs by default)
// NetBSD: readlink /proc/curproc/exe
// DragonFly BSD: readlink /proc/curproc/file
// Windows: GetModuleFileName() with hModule = NULL
std::filesystem::path path() {
#ifdef _WIN32
  DWORD size = 0;
  std::wstring str;
  do {
    str.resize(str.size() + MAX_PATH);
    size = GetModuleFileName(nullptr, &str[0], static_cast<DWORD>(str.size()));
  } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);
  str.resize(size);
  return std::filesystem::canonical(str);
#endif
#ifdef __linux__
  std::filesystem::path path;
  if (auto str = realpath("/proc/self/exe", nullptr)) {
    try {
      path = std::filesystem::u8path(str);
      free(str);
    }
    catch (...) {
      free(str);
    }
  }
  return path;
#endif
#ifdef __FreeBSD__
  int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
  size_t size = 0;
  if (sysctl(mib, 4, nullptr, &size, nullptr, 0) == 0) {
    std::string str;
    str.resize(size);
    if (sysctl(mib, 4, &str[0], &size, nullptr, 0) == 0) {
      str.resize(size > 0 ? size - 1 : 0);
      return std::filesystem::u8path(str);
    }
  }
  return {};
#endif
}

}  // namespace application
}  // namespace ice
