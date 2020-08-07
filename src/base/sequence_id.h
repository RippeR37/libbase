#pragma once

#include <cstdint>

namespace base {

namespace detail {
class SequenceIdGenerator;
}

/**
 * This class represents an id of a sequence for SequencedTaskRunner.
 */
class SequenceId {
 public:
  SequenceId(const SequenceId&) = default;

  bool operator==(const SequenceId& other) const;

 private:
  friend class detail::SequenceIdGenerator;

  SequenceId(uint64_t id);

  uint64_t id_;
};

}  // namespace base
