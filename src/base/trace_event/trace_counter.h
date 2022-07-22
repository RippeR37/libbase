#pragma once

#ifndef LIBBASE_TRACING_DISABLE

#include "base/trace_event/trace_event_register.h"

#define TRACE_COUNTER(categories, name, arg1_key, arg1_value, ...)           \
  ::base::detail::EventRegister::RegisterCounter(categories, name, arg1_key, \
                                                 arg1_value, ##__VA_ARGS__)

#define TRACE_COUNTER_ID(categories, name, id, arg1_key, arg1_value, ...) \
  ::base::detail::EventRegister::RegisterCounterId(                       \
      categories, name, id, arg1_key, arg1_value, ##__VA_ARGS__)

#else

#define TRACE_COUNTER(categories, name, arg1_key, arg1_value, ...)
#define TRACE_COUNTER_ID(categories, name, id, arg1_key, arg1_value, ...)

#endif
