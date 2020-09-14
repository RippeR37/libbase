#pragma once

#include <optional>

#include "base/logging.h"
#include "base/sequenced_task_runner_helpers.h"

namespace base {

#if DCHECK_IS_ON()

#define SEQUENCE_CHECKER(name) ::base::SequenceChecker name
#define DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker) \
  DCHECK(sequence_checker.CalledOnValidSequence());
#define DETACH_FROM_SEQUENCE(sequence_checker) \
  (sequence_checker).DetachFromSequence()

#else

#define SEQUENCE_CHECKER(name) static_assert(true, "")
#define DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker)
#define DETACH_FROM_SEQUENCE(sequence_checker)

#endif

class SequenceChecker {
 public:
  SequenceChecker();
  ~SequenceChecker();

  SequenceChecker(SequenceChecker&& other);
  SequenceChecker& operator=(SequenceChecker&& other);

  bool CalledOnValidSequence() const;
  void DetachFromSequence();

 private:
  mutable std::optional<SequenceId> sequence_id_;
};

}  // namespace base
