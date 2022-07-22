#include "base/trace_event/trace_platform.h"

#if LIBBASE_IS_LINUX || LIBBASE_IS_MACOS
#include <sys/syscall.h>
#include <unistd.h>
#elif LIBBASE_IS_WINDOWS
#include <process.h>
#include <windows.h>
#endif

namespace base {
namespace detail {

uint64_t TracePlatform::GetPid() {
#if LIBBASE_IS_LINUX || LIBBASE_IS_MACOS
  return ::getpid();
#elif LIBBASE_IS_WINDOWS
  return ::GetCurrentProcessId();
#endif
  return -1;
}

uint64_t TracePlatform::GetTid() {
#if LIBBASE_IS_LINUX || LIBBASE_IS_MACOS
  return ::syscall(SYS_gettid);
#elif LIBBASE_IS_WINDOWS
  return ::GetCurrentThreadId();
#endif
  return -1;
}

}  // namespace detail
}  // namespace base
