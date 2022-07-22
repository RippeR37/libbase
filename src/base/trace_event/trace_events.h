#pragma once

#ifndef LIBBASE_TRACING_DISABLE

#include <cstdint>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "base/trace_event/trace_argument_packer.h"

namespace base {
namespace detail {

struct TraceEvent {
  using Arguments = ArgumentPacker::StringArguments;

  std::string name;
  std::string cat;
  std::string id;
  char ph;
  uint64_t ts;
  uint64_t pid;
  uint64_t tid;
  Arguments args;

  void WriteTo(std::ostream& stream) const;
};

struct TraceCompleteEvent {
  using Arguments = ArgumentPacker::StringArguments;

  std::string name;
  std::string cat;
  uint64_t ts;
  uint64_t dur;
  uint64_t pid;
  uint64_t tid;
  Arguments args;

  void WriteTo(std::ostream& stream) const;
};

struct TraceCounter {
  using Arguments = ArgumentPacker::IntegerArguments;

  std::string name;
  std::string cat;
  uint64_t ts;
  uint64_t pid;
  Arguments args;

  void WriteTo(std::ostream& stream) const;
};

struct TraceCounterId {
  using Arguments = TraceCounter::Arguments;

  std::string name;
  std::string cat;
  std::string id;
  uint64_t ts;
  uint64_t pid;
  Arguments args;

  void WriteTo(std::ostream& stream) const;
};

struct TraceInstantEvent {
  using Arguments = ArgumentPacker::StringArguments;

  std::string name;
  std::string cat;
  char s;
  uint64_t ts;
  uint64_t pid;
  uint64_t tid;
  Arguments args;

  void WriteTo(std::ostream& stream) const;
};

}  // namespace detail
}  // namespace base

#endif
