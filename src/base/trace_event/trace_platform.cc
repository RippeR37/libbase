#ifdef LIBBASE_ENABLE_TRACING

#include "base/trace_event/trace_platform.h"

#if defined(LIBBASE_IS_LINUX) || defined(LIBBASE_IS_MACOS)
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(LIBBASE_IS_WINDOWS)
#include <process.h>
#include <Windows.h>
#endif

namespace base {
namespace detail {

uint64_t TracePlatform::GetPid() {
#if defined(LIBBASE_IS_LINUX) || defined(LIBBASE_IS_MACOS)
  return ::getpid();
#elif defined(LIBBASE_IS_WINDOWS)
  return ::GetCurrentProcessId();
#else
  return static_cast<uint64_t>(-1);
#endif
}

uint64_t TracePlatform::GetTid() {
#if defined(LIBBASE_IS_LINUX) || defined(LIBBASE_IS_MACOS)
  return ::syscall(SYS_gettid);
#elif defined(LIBBASE_IS_WINDOWS)
  return ::GetCurrentThreadId();
#else
  return static_cast<uint64_t>(-1);
#endif
}

}  // namespace detail
}  // namespace base

#endif
