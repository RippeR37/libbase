#pragma once

#ifndef LIBBASE_TRACING_DISABLE

#include "base/trace_event/trace_event_register.h"

#define TRACE_EVENT_ASYNC_BEGIN(categories, name, id, ...)                     \
  ::base::detail::EventRegister::RegisterAsyncEvent(categories, name, id, 'b', \
                                                    ##__VA_ARGS__)

#define TRACE_EVENT_ASYNC_STEP(categories, name, id, ...)                      \
  ::base::detail::EventRegister::RegisterAsyncEvent(categories, name, id, 'n', \
                                                    ##__VA_ARGS__)

#define TRACE_EVENT_END(categories, name, id, ...)                             \
  ::base::detail::EventRegister::RegisterAsyncEvent(categories, name, id, 'e', \
                                                    ##__VA_ARGS__)

#else

#define TRACE_EVENT_ASYNC_BEGIN(categories, name, id, ...)
#define TRACE_EVENT_ASYNC_STEP(categories, name, id, ...)
#define TRACE_EVENT_ASYNC_END(categories, name, id, ...)

#endif
