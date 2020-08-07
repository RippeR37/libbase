#include "base/sequence_id.h"

namespace base {

SequenceId::SequenceId(uint64_t id) : id_(id) {}

bool SequenceId::operator==(const SequenceId& other) const {
  return (id_ == other.id_);
}

bool SequenceId::operator!=(const SequenceId& other) const {
  return !(*this == other);
}

}  // namespace base
