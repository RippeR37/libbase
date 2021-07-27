#pragma once

#include "base/sequenced_task_runner.h"

namespace base {

class SingleThreadTaskRunner : public SequencedTaskRunner {
 public:
  // Alias for `RunsTasksInCurrentSequence()`.
  bool BelongsToCurrentThread() const { return RunsTasksInCurrentSequence(); }
};

}  // namespace base
