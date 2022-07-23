#pragma once

#ifdef LIBBASE_ENABLE_TRACING

#include "base/trace_event/trace_event_register.h"

#define TRACE_SIGNAL(categories, name, ...)                                  \
  ::base::detail::EventRegister::RegisterInstantEvent(categories, name, 'g', \
                                                      ##__VA_ARGS__)

#define TRACE_SIGNAL_THREAD(categories, name, ...)                           \
  ::base::detail::EventRegister::RegisterInstantEvent(categories, name, 't', \
                                                      ##__VA_ARGS__)

#define TRACE_SIGNAL_PROCESS(categories, name, ...)                          \
  ::base::detail::EventRegister::RegisterInstantEvent(categories, name, 'p', \
                                                      ##__VA_ARGS__)

#else

#define TRACE_SIGNAL(categories, name, ...)
#define TRACE_SIGNAL_THREAD(categories, name, ...)
#define TRACE_SIGNAL_PROCESS(categories, name, ...)

#endif
