#pragma once

#include "base/sequenced_task_runner.h"

namespace base {

class SingleThreadTaskRunner : public SequencedTaskRunner {
 public:
  virtual ~SingleThreadTaskRunner() = default;

  // Alias for `RunsTasksInCurrentSequence()`.
  bool BelongsToCurrentThread() const { return RunsTasksInCurrentSequence(); }
};

}  // namespace base
