#pragma once

#include <string>

namespace base {

class SourceLocation {
 public:
  SourceLocation(std::string file, size_t line)
      : file(std::move(file)), line(line) {}

  std::string file;
  size_t line;
};

}  // namespace base

#define FROM_HERE \
  ::base::SourceLocation { __FILE__, __LINE__ }
