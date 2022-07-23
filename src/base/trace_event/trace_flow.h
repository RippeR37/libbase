#pragma once

#ifdef LIBBASE_ENABLE_TRACING

#include "base/trace_event/trace_event_register.h"

#define TRACE_EVENT_WITH_FLOW_BEGIN(categories, name, id, ...)                 \
  ::base::detail::EventRegister::RegisterAsyncEvent(categories, name, id, 's', \
                                                    ##__VA_ARGS__)

#define TRACE_EVENT_WITH_FLOW_STEP(categories, name, id, ...)                  \
  ::base::detail::EventRegister::RegisterAsyncEvent(categories, name, id, 't', \
                                                    ##__VA_ARGS__)

#define TRACE_EVENT_WITH_FLOW_END(categories, name, id, ...)                   \
  ::base::detail::EventRegister::RegisterAsyncEvent(categories, name, id, 'f', \
                                                    ##__VA_ARGS__)

#else

#define TRACE_EVENT_WITH_FLOW_BEGIN(categories, name, id, ...)
#define TRACE_EVENT_WITH_FLOW_STEP(categories, name, id, ...)
#define TRACE_EVENT_WITH_FLOW_END(categories, name, id, ...)

#endif
