#pragma once

#include <cstddef>

namespace base {

class SourceLocation {
 public:
  SourceLocation(const char* file, size_t line) : file(file), line(line) {}

  const char* file;
  size_t line;
};

}  // namespace base

#define FROM_HERE \
  ::base::SourceLocation { __FILE__, __LINE__ }
