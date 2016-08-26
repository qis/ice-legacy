#include <ice/stack.h>
#include <assert.h>
#if defined(__linux__)
#include <cxxabi.h>
#include <execinfo.h>
#endif
#include <signal.h>
#include <sstream>
#include <string>

namespace ice {
namespace stack {
namespace {

std::string g_trace;
crash_handler g_handler;

void signal_handler(int sig, siginfo_t* siginfo, void* context)
{
  if (!g_handler) {
    _Exit(1);
  }

  std::ostringstream oss;

  switch(sig) {
  case SIGSEGV:
    oss << "SIGSEGV: Segmentation fault.";
    break;
  case SIGINT:
    oss << "SIGINT: Interactive attention signal (e.g. CTRL+C).";
    break;
  case SIGFPE:
    switch(siginfo->si_code) {
    case FPE_INTDIV:
      oss << "SIGFPE: Integer divide by zero.";
      break;
    case FPE_INTOVF:
      oss << "SIGFPE: Integer overflow.";
      break;
    case FPE_FLTDIV:
      oss << "SIGFPE: Floating point divide by zero.";
      break;
    case FPE_FLTOVF:
      oss << "SIGFPE: Floating point overflow.";
      break;
    case FPE_FLTUND:
      oss << "SIGFPE: Floating point underflow.";
      break;
    case FPE_FLTRES:
      oss << "SIGFPE: Floating point inexact result.";
      break;
    case FPE_FLTINV:
      oss << "SIGFPE: Floating point invalid operation.";
      break;
    case FPE_FLTSUB:
      oss << "SIGFPE: Subscript out of range.";
      break;
    default:
      oss << "SIGFPE: Arithmetic exception.";
      break;
    }
  case SIGILL:
    switch(siginfo->si_code) {
    case ILL_ILLOPC:
      oss << "SIGILL: Illegal opcode.";
      break;
    case ILL_ILLOPN:
      oss << "SIGILL: Illegal operand.";
      break;
    case ILL_ILLADR:
      oss << "SIGILL: Illegal addressing mode.";
      break;
    case ILL_ILLTRP:
      oss << "SIGILL: Illegal trap.";
      break;
    case ILL_PRVOPC:
      oss << "SIGILL: Privileged opcode.";
      break;
    case ILL_PRVREG:
      oss << "SIGILL: Privileged register.";
      break;
    case ILL_COPROC:
      oss << "SIGILL: Coprocessor error.";
      break;
    case ILL_BADSTK:
      oss << "SIGILL: Internal stack error.";
      break;
    default:
      oss << "SIGILL: Illegal instruction.";
      break;
    }
    break;
  case SIGTERM:
    oss << "SIGTERM: A termination request was received.";
    break;
  case SIGABRT:
    oss << "SIGABRT: Application aborted.";
    break;
  default:
    oss << "Unsupported signal.";
    break;
  }

#if defined(__linux__)
  oss << '\n';

  // Storage array for stack trace address data.
  const size_t max_frames = 65;
  void* addrlist[max_frames + 1];

  // Retrieve current stack addresses.
  if (auto addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*))) {
    // Resolve addresses into strings containing "filename(function+address)".
    if (auto symbollist = backtrace_symbols(addrlist, addrlen)) {
      // Allocate string which will be filled with the demangled function name.
      size_t funcnamesize = 256;
      auto funcname = reinterpret_cast<char*>(malloc(funcnamesize));

      // Iterate over the returned symbol lines. Skip the first.
      for (decltype(addrlen) i = 1; i < addrlen; i++) {
        char* begin_name = nullptr;
        char* begin_offset = nullptr;
        char* end_offset = nullptr;

        // Find parentheses and +address offset surrounding the mangled name:
        //  ./module : function+0x15c + 0x8048a6d
        for (auto p = symbollist[i]; *p; ++p) {
          if (*p == '(') {
            begin_name = p;
          } else if (*p == '+') {
            begin_offset = p;
          } else if (*p == ')' && begin_offset) {
            end_offset = p;
            break;
          }
        }

        if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
          *begin_name++ = '\0';
          *begin_offset++ = '\0';
          *end_offset = '\0';

          // The mangled name is now in [begin_name, begin_offset) and caller
          // offset in [begin_offset, end_offset). now apply __cxa_demangle().
          auto status = 0;
          auto str = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
          if (status == 0) {
            oss << symbollist[i] << " : " << str << " [" << begin_offset << "]\n"; // use possibly realloc()-ed string
          }
          else {
            // Demangling failed. Output function name as a C function with no arguments.
            oss << symbollist[i] << " : " << begin_name << "() [" << begin_offset << "]\n";
          }
        } else {
          // Print the whole line on parse error.
          for (auto p = symbollist[i]; *p; ++p) {
            if (*p == '(') {
              *p = ' ';
            } else if (*p == ')') {
              *p = ':';
              break;
            }
          }
          oss << symbollist[i] << '\n';
        }
      }

      free(funcname);
      free(symbollist);
    }
  }
#endif

  g_trace = oss.str();
  g_handler(g_trace.c_str());
  _Exit(1);
}

}  // namespace

void set_crash_handler(crash_handler handler)
{
  g_handler = handler;

  struct sigaction sig_action = {};
  sig_action.sa_sigaction = signal_handler;
  sigemptyset(&sig_action.sa_mask);
  sig_action.sa_flags = SA_SIGINFO;
 
  if (sigaction(SIGSEGV, &sig_action, NULL) != 0) {
    fputs("Could not set SIGSEGV handler.\n", stderr);
  }
  if (sigaction(SIGFPE,  &sig_action, NULL) != 0) {
    fputs("Could not set SIGFPE handler.\n", stderr);
  }
  if (sigaction(SIGINT,  &sig_action, NULL) != 0) {
    fputs("Could not set SIGINT handler.\n", stderr);
  }
  if (sigaction(SIGILL,  &sig_action, NULL) != 0) {
    fputs("Could not set SIGILL handler.\n", stderr);
  }
  if (sigaction(SIGTERM, &sig_action, NULL) != 0) {
    fputs("Could not set SIGTERM handler.\n", stderr);
  }
  if (sigaction(SIGABRT, &sig_action, NULL) != 0) {
    fputs("Could not set SIGABRT handler\n", stderr);
  }
}

crash_handler get_crash_handler()
{
  return g_handler;
}

}  // namespace stack
}  // namespace ice
