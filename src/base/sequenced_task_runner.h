#pragma once

#include "base/task_runner.h"

namespace base {

class SequencedTaskRunner : public TaskRunner {
 public:
  // Returns true if:
  // - this is a SequencedTaskRunner to which the current task was posted,
  // - this is a SequencedTaskRunner bound to the same sequence as the
  //   SequencedTaskRunner to which the current task was posted,
  // - this is a SingleThreadTaskRunner bound to the current thread.
  virtual bool RunsTasksInCurrentSequence() const = 0;
};

}  // namespace base
