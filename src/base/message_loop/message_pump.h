#pragma once

#include <memory>
#include <optional>

#include "base/callback.h"
#include "base/sequence_id.h"

namespace base {

class SequencedTaskRunner;

class MessagePump {
 public:
  using ExecutorId = uintptr_t;  // TODO: make it better!

  struct PendingTask {
    explicit operator bool() const { return !!task; }

    OnceClosure task;
    std::optional<SequenceId> sequence_id;
    std::optional<ExecutorId> allowed_executor_id;
    std::weak_ptr<SequencedTaskRunner> target_task_runner;
  };

  virtual ~MessagePump() = default;

  virtual PendingTask GetNextPendingTask(ExecutorId executor_id,
                                         bool wait_for_task) = 0;
  virtual bool QueuePendingTask(PendingTask pending_task) = 0;

  virtual void Stop(PendingTask last_task) = 0;
};

}  // namespace base
