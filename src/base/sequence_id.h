#pragma once

#include <cstdint>

namespace base {

/**
 * This class represents an Id of a sequence for SequencedTaskRunner.
 */
class SequenceId {
 public:
  SequenceId(uint64_t id);
  bool operator==(const SequenceId& other) const;

 private:
  uint64_t id_;
};

}  // namespace base
