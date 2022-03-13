#pragma once

#include <condition_variable>
#include <list>
#include <mutex>
#include <optional>
#include <vector>

#include "base/message_loop/message_pump.h"

namespace base {

class MessagePumpImpl : public MessagePump {
 public:
  explicit MessagePumpImpl(size_t executors_count);

  // MessagePump
  PendingTask GetNextPendingTask(ExecutorId executor_id) override;
  bool QueuePendingTask(PendingTask pending_task) override;
  void Stop(PendingTask last_task) override;

 private:
  using PendingTaskList = std::list<PendingTask>;
  using PendingTaskIter = PendingTaskList::iterator;

  PendingTask GetNextPendingTask_Locked(ExecutorId executor_id);
  bool IsTaskFromSequenceCurrentlyProcessed_Locked(
      SequenceId sequence_id) const;
  bool HasAllowedPendingTasks_Locked(ExecutorId executor_id);
  PendingTaskIter FindFirstAllowedPendingTaskIter_Locked(
      ExecutorId executor_id);

  std::mutex mutex_;
  std::condition_variable cond_var_;
  bool stopped_;
  PendingTaskList pending_tasks_;  // TODO: move to separate class
  std::vector<std::optional<SequenceId>> active_sequences_;
};

}  // namespace base
