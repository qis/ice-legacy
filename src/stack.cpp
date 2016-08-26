#if defined(_MSC_VER)
#include "stack_msvc.h"
#elif defined(__GNUC__)
#include "stack_gcc.h"
#endif
