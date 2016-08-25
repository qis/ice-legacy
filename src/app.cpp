#include <ice/app.h>
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
namespace app {

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
std::filesystem::path file()
{
#ifdef _WIN32
  DWORD size = 0;
  std::wstring path;
  do {
    path.resize(path.size() + MAX_PATH);
    size = GetModuleFileName(nullptr, &path[0], static_cast<DWORD>(path.size()));
  } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);
  path.resize(size);
  return std::filesystem::canonical(path);
#endif
#ifdef __linux__
  if (auto str = realpath("/proc/self/exe", nullptr)) {
    std::string path(str);
    free(str);
    return path;
  }
  return{};
#endif
#ifdef __FreeBSD__
  int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
  size_t size = 0;
  if (sysctl(mib, 4, nullptr, &size, nullptr, 0) == 0) {
    std::string path;
    path.resize(size);
    if (sysctl(mib, 4, &path[0], &size, nullptr, 0) == 0) {
      path.resize(size > 0 ? size - 1 : 0);
      return path;
    }
  }
  return{};
#endif
}

std::filesystem::path path()
{
  return file().parent_path();
}

}  // namespace app
}  // namespace ice
