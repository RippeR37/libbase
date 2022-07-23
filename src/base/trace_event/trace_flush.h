#pragma once

#ifdef LIBBASE_ENABLE_TRACING

#include "base/trace_event/trace_event_register.h"

#define TRACE_EVENT_FLUSH_TO_FILE(file) \
  ::base::detail::EventRegister::FlushEventsToFile(file)

#define TRACE_EVENT_FLUSH_TO_STREAM(stream) \
  ::base::detail::EventRegister::FlushEventsToStream(stream)

#else

#define TRACE_EVENT_FLUSH_TO_FILE(file)
#define TRACE_EVENT_FLUSH_TO_STREAM(stream)

#endif
