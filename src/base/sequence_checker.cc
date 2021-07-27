#include "base/sequence_checker.h"

namespace base {

SequenceChecker::SequenceChecker()
    : sequence_id_(detail::CurrentSequenceIdHelper::current_sequence_id_) {}

SequenceChecker::~SequenceChecker() = default;

SequenceChecker::SequenceChecker(SequenceChecker&& other) {
  const bool other_moved_on_valid_sequence = other.CalledOnValidSequence();
  CHECK(other_moved_on_valid_sequence);

  sequence_id_ = std::exchange(other.sequence_id_, std::nullopt);
}

SequenceChecker& SequenceChecker::operator=(SequenceChecker&& other) {
  CHECK(CalledOnValidSequence());

  const bool other_moved_on_valid_sequence = other.CalledOnValidSequence();
  CHECK(other_moved_on_valid_sequence);

  sequence_id_ = std::exchange(other.sequence_id_, std::nullopt);

  return *this;
}

bool SequenceChecker::CalledOnValidSequence() const {
  if (!sequence_id_) {
    sequence_id_ = detail::CurrentSequenceIdHelper::current_sequence_id_;
  }

  CHECK(sequence_id_) << "Must run while in sequence!";
  return (sequence_id_ ==
          detail::CurrentSequenceIdHelper::current_sequence_id_);
}

void SequenceChecker::DetachFromSequence() {
  if (!sequence_id_) {
    return;
  }

  CHECK(CalledOnValidSequence());
  sequence_id_.reset();
}

}  // namespace base
