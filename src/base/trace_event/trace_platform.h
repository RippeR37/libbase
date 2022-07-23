#pragma once

#ifdef LIBBASE_ENABLE_TRACING

#include <cstdint>
#include <sstream>
#include <thread>

namespace base {
namespace detail {

class TracePlatform {
 public:
  static uint64_t GetPid();
  static uint64_t GetTid();
};

}  // namespace detail
}  // namespace base

#endif
