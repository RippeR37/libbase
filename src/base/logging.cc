#include "base/logging.h"

#include <iomanip>
#include <iostream>

namespace base {
namespace detail {

void LogFormatter(std::ostream& stream,
                  const google::LogMessageInfo& log,
                  void*) {
  // Chromium log format:
  //
  //   [PID:TID:MMDD/TIME:LEVEL:FILE_NAME(LINE_NUMBER)]
  //
  // `PID` part is ignored for now since `libbase` doesn't include any IPC or
  // cross-process functionality.

  stream << '[' << log.thread_id << ':';
  stream << std::setw(2) << log.time.month() + 1;
  stream << std::setw(2) << log.time.day() << '/';
  stream << std::setw(2) << log.time.hour();
  stream << std::setw(2) << log.time.min();
  stream << std::setw(2) << log.time.sec() << '.';
  stream << std::setw(6) << log.time.usec() << ':';
  stream << log.severity << ':';
  stream << log.filename << '(' << log.line_number << ")]";
}

}  // namespace detail
}  // namespace base
