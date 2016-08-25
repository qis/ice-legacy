#include <ice/stack.h>
#include <windows.h>
#include <array>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>

#pragma warning(push)
#pragma warning(disable: 4091)
#include <dbghelp.h>
#pragma warning(pop)

namespace ice {
namespace stack {
namespace {

std::string g_trace;
crash_handler g_handler;

template<std::size_t S>
struct symbol : public SYMBOL_INFO {
  symbol() : data({})
  {
    SizeOfStruct = sizeof(SYMBOL_INFO);
    MaxNameLen = S;
  }

  std::array<std::uint8_t, S> data;
};

#define LOAD_FUNCTION(module, name)                                                \
  auto name = reinterpret_cast<decltype(&::name)>(GetProcAddress(module, #name));  \
  if (!name) {                                                                     \
    g_handler("Could not get proc address: " # name);                              \
    return EXCEPTION_EXECUTE_HANDLER;                                              \
  }

LONG WINAPI unhandled_exception_filter(PEXCEPTION_POINTERS ep)
{
  if (!g_handler) {
    return EXCEPTION_EXECUTE_HANDLER;
  }

  auto module = LoadLibrary(L"dbghelp.dll");
  if (!module) {
    g_handler("Could not load library: dbghelp.dll");
    return EXCEPTION_EXECUTE_HANDLER;
  }

  LOAD_FUNCTION(module, SymInitialize);
  LOAD_FUNCTION(module, SymFunctionTableAccess64);
  LOAD_FUNCTION(module, SymGetModuleBase64);
  LOAD_FUNCTION(module, SymFromAddr);
  LOAD_FUNCTION(module, SymGetModuleInfo64);
  LOAD_FUNCTION(module, StackWalk64);

  auto er = ep->ExceptionRecord;
  auto ec = er->ExceptionCode;
  auto cr = *ep->ContextRecord;

  auto process = GetCurrentProcess();
  auto thread = GetCurrentThread();

  SymInitialize(process, NULL, TRUE);

  STACKFRAME64 stack_frame = {};
#if defined(_WIN64)
  int machine_type = IMAGE_FILE_MACHINE_AMD64;
  stack_frame.AddrPC.Offset = cr.Rip;
  stack_frame.AddrFrame.Offset = cr.Rbp;
  stack_frame.AddrStack.Offset = cr.Rsp;
#else
  int machine_type = IMAGE_FILE_MACHINE_I386;
  stack_frame.AddrPC.Offset = cr.Eip;
  stack_frame.AddrFrame.Offset = cr.Ebp;
  stack_frame.AddrStack.Offset = cr.Esp;
#endif
  stack_frame.AddrPC.Mode = AddrModeFlat;
  stack_frame.AddrFrame.Mode = AddrModeFlat;
  stack_frame.AddrStack.Mode = AddrModeFlat;

  std::ostringstream oss;
  oss << std::setfill('0') << std::hex;
  while (StackWalk64(machine_type, process, thread, &stack_frame, &cr, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
    symbol<1024> symbol;
    DWORD64 displacement = 0;
    if (SymFromAddr(process, (DWORD64)stack_frame.AddrPC.Offset, &displacement, &symbol)) {
      IMAGEHLP_MODULE64 mi = {};
      mi.SizeOfStruct = sizeof(mi);
      if (SymGetModuleInfo64(process, symbol.ModBase, &mi)) {
        oss << mi.ModuleName << ": ";
      }
      oss << symbol.Name << " + 0x" << displacement << '\n';
    }
  }
  g_trace = oss.str();
  g_handler(g_trace.c_str());
  return EXCEPTION_EXECUTE_HANDLER;
}

}  // namespace

void set_crash_handler(crash_handler handler)
{
  g_handler = handler;
  SetUnhandledExceptionFilter(unhandled_exception_filter);
}

crash_handler get_crash_handler()
{
  return g_handler;
}

}  // namespace stack
}  // namespace ice
