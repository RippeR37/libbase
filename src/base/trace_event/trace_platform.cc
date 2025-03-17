#ifdef LIBBASE_ENABLE_TRACING

#include "base/trace_event/trace_platform.h"

#if defined(LIBBASE_IS_LINUX) || defined(LIBBASE_IS_MACOS)
#include <sys/syscall.h>
#include <unistd.h>
#if defined(LIBBASE_IS_MACOS) && defined(__has_include) && \
    __has_include(<pthread.h>)
#include <pthread.h>
#endif
#elif defined(LIBBASE_IS_WINDOWS)
#include <Windows.h>
#include <process.h>
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
#if defined(LIBBASE_IS_MACOS) && defined(__has_include) && \
    __has_include(<pthread.h>)
  uint64_t tid;
  const int error = pthread_threadid_np(nullptr, &tid);
  return error ? -1 : tid;
#else
  return ::syscall(SYS_gettid);
#endif
#elif defined(LIBBASE_IS_WINDOWS)
  return ::GetCurrentThreadId();
#else
  return static_cast<uint64_t>(-1);
#endif
}

}  // namespace detail
}  // namespace base

#endif
