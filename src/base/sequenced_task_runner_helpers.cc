#include "base/sequenced_task_runner_helpers.h"

#include "base/logging.h"

namespace base {
namespace detail {

// static
SequenceId SequenceIdGenerator::GetNextSequenceId() {
  static std::atomic_uint64_t next_id = 0;
  return SequenceId{next_id++};
}

// static
bool CurrentSequenceIdHelper::IsCurrentSequence(const SequenceId& sequence_id) {
  return current_sequence_id_ && (*current_sequence_id_ == sequence_id);
}

ScopedSequenceIdSetter::ScopedSequenceIdSetter(SequenceId current_sequence_id) {
  CHECK(!CurrentSequenceIdHelper::current_sequence_id_);
  CurrentSequenceIdHelper::current_sequence_id_ = current_sequence_id;
}

ScopedSequenceIdSetter::~ScopedSequenceIdSetter() {
  CHECK(CurrentSequenceIdHelper::current_sequence_id_);
  CurrentSequenceIdHelper::current_sequence_id_.reset();
}

}  // namespace detail
}  // namespace base
