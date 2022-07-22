#pragma once

#ifndef LIBBASE_TRACING_DISABLE

#include <algorithm>
#include <string>

#include "base/trace_event/trace_argument_packer.h"
#include "base/trace_event/trace_event_register.h"

namespace base {
namespace detail {

class ScopedTraceEvent {
 public:
  template <typename... Args>
  ScopedTraceEvent(std::string categories, std::string name, Args&&... args)
      : categories_(std::move(categories)), name_(std::move(name)) {
    EventRegister::RegisterEvent(categories_, name_, 'B',
                                 std::forward<Args>(args)...);
  }

  ~ScopedTraceEvent() {
    EventRegister::RegisterEvent(std::move(categories_), std::move(name_), 'E');
  }

 private:
  std::string categories_;
  std::string name_;
};

}  // namespace detail
}  // namespace base

#define TRACE_VAR_NAME_CONCAT_IMPL(a, b) a##b
#define TRACE_VAR_NAME_CONCAT(a, b) TRACE_VAR_NAME_CONCAT_IMPL(a, b)

#define TRACE_EVENT(categories, name, ...)                \
  ::base::detail::ScopedTraceEvent TRACE_VAR_NAME_CONCAT( \
      libbase_scoped_trace_event_, __COUNTER__) {         \
    categories, name, ##__VA_ARGS__                       \
  }

#define TRACE_EVENT_BEGIN(categories, name, ...)                      \
  ::base::detail::EventRegister::RegisterEvent(categories, name, 'B', \
                                               ##__VA_ARGS__)

#define TRACE_EVENT_END(categories, name, ...)                        \
  ::base::detail::EventRegister::RegisterEvent(categories, name, 'E', \
                                               ##__VA_ARGS__)

#else

#define TRACE_EVENT(categories, name, ...)
#define TRACE_EVENT_BEGIN(categories, name, ...)
#define TRACE_EVENT_END(categories, name, ...)

#endif
