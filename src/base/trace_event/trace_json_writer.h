#pragma once

#ifdef LIBBASE_ENABLE_TRACING

#include <cstdint>
#include <list>
#include <string>

#include "base/trace_event/trace_events.h"

namespace base {
namespace detail {

class JsonWriter {
 public:
  static void WriteAll(std::ostream& stream,
                       const std::list<TraceEvent>& generic_events,
                       const std::list<TraceCompleteEvent>& complete_events,
                       const std::list<TraceCounter>& counter_events,
                       const std::list<TraceCounterId>& counter_id_events,
                       const std::list<TraceInstantEvent>& instant_events) {
    stream << "{\"traceEvents\":[\n";
    bool written_data = false;
    WriteEvents(stream, generic_events, written_data);
    WriteEvents(stream, complete_events, written_data);
    WriteEvents(stream, counter_events, written_data);
    WriteEvents(stream, counter_id_events, written_data);
    WriteEvents(stream, instant_events, written_data);
    stream << "\n]}";
  }

 private:
  template <typename T>
  static void WriteEvents(std::ostream& stream,
                          const T& events,
                          bool& written_data) {
    for (const auto& current_event : events) {
      if (written_data) {
        stream << ",\n";
      }
      current_event.WriteTo(stream);
      written_data = true;
    }
  }
};

}  // namespace detail
}  // namespace base

#endif
