#pragma once

#include <optional>

#include "base/callback.h"
#include "base/sequence_id.h"

namespace base {

class MessagePump {
 public:
  using ExecutorId = uintptr_t;  // TODO: make it better!

  struct PendingTask {
    operator bool() const { return !!task; }

    OnceClosure task;
    std::optional<SequenceId> sequence_id;
    std::optional<ExecutorId> allowed_executor_id;
  };

  virtual ~MessagePump() = default;

  virtual PendingTask GetNextPendingTask(ExecutorId executor_id) = 0;
  virtual void QueuePendingTask(PendingTask pending_task) = 0;

  virtual void Stop() = 0;
};

}  // namespace base
