#pragma once

#ifdef LIBBASE_ENABLE_TRACING

#include "base/trace_event/trace_event_register.h"

#define TRACE_EVENT_COMPLETE(categories, name, duration, ...) \
  ::base::detail::EventRegister::RegisterCompleteEvent(       \
      categories, name, duration, ##__VA_ARGS__)

#else

#define TRACE_EVENT_COMPLETE(categories, name, duration, ...)

#endif
