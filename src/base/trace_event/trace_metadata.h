#pragma once

#ifdef LIBBASE_ENABLE_TRACING

#include "base/trace_event/trace_event_register.h"

#define TRACE_NAME_PROCESS(name)                                        \
  ::base::detail::EventRegister::RegisterEvent("", "process_name", 'M', \
                                               "name", name)

#define TRACE_NAME_THREAD(name)                                                \
  ::base::detail::EventRegister::RegisterEvent("", "thread_name", 'M', "name", \
                                               name)

#else

#define TRACE_NAME_PROCESS(name)
#define TRACE_NAME_THREAD(name)

#endif
