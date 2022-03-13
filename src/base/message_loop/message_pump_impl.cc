#include "base/message_loop/message_pump_impl.h"

#include <algorithm>

#include "base/logging.h"

namespace base {

MessagePumpImpl::MessagePumpImpl(size_t executors_count) : stopped_(false) {
  active_sequences_.resize(executors_count);
}

MessagePumpImpl::PendingTask MessagePumpImpl::GetNextPendingTask(
    ExecutorId executor_id) {
  std::unique_lock<std::mutex> lock(mutex_);

  // Executor asks for a next pending task only if it finished processing last
  // one. Based on that we can unblock processing of tasks from the same
  // sequence the last executor's task was.
  DCHECK_LT(executor_id, active_sequences_.size());
  active_sequences_[executor_id].reset();

  if (auto pending_task = GetNextPendingTask_Locked(executor_id)) {
    return pending_task;
  }

  cond_var_.wait(lock, [&]() {
    return (stopped_ || HasAllowedPendingTasks_Locked(executor_id));
  });
  return GetNextPendingTask_Locked(executor_id);
}

bool MessagePumpImpl::QueuePendingTask(PendingTask pending_task) {
  bool task_queued = false;

  {
    std::lock_guard<std::mutex> guard(mutex_);
    if (!stopped_) {
      pending_tasks_.push_back(std::move(pending_task));
      task_queued = true;
    }
  }

  cond_var_.notify_one();

  return task_queued;
}

void MessagePumpImpl::Stop(PendingTask last_task) {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    if (!stopped_ && last_task) {
      pending_tasks_.push_back(std::move(last_task));
    }
    stopped_ = true;
  }

  cond_var_.notify_all();
}

MessagePumpImpl::PendingTask MessagePumpImpl::GetNextPendingTask_Locked(
    ExecutorId executor_id) {
  const auto allowed_pending_task_iter =
      FindFirstAllowedPendingTaskIter_Locked(executor_id);
  if (allowed_pending_task_iter == pending_tasks_.end()) {
    return {};
  }

  PendingTask task = std::move(*allowed_pending_task_iter);
  pending_tasks_.erase(allowed_pending_task_iter);

  // Mark that requesting executor is now processing task from given sequence.
  active_sequences_[executor_id] = task.sequence_id;

  return task;
}

bool MessagePumpImpl::HasAllowedPendingTasks_Locked(ExecutorId executor_id) {
  return FindFirstAllowedPendingTaskIter_Locked(executor_id) !=
         pending_tasks_.end();
}

MessagePumpImpl::PendingTaskIter
MessagePumpImpl::FindFirstAllowedPendingTaskIter_Locked(
    ExecutorId executor_id) {
  // Pending task is allowed for a given executor if:
  // - task is assigned to that executor or is not assigned to any,
  // - no executor is currently processing a task from the same sequence.
  const auto is_pending_task_allowed = [&](const PendingTask& task) {
    return (task.allowed_executor_id.value_or(executor_id) == executor_id) &&
           (!task.sequence_id ||
            !IsTaskFromSequenceCurrentlyProcessed_Locked(*task.sequence_id));
  };

  return std::find_if(pending_tasks_.begin(), pending_tasks_.end(),
                      is_pending_task_allowed);
}

bool MessagePumpImpl::IsTaskFromSequenceCurrentlyProcessed_Locked(
    SequenceId sequence_id) const {
  return std::any_of(active_sequences_.begin(), active_sequences_.end(),
                     [sequence_id](const auto& active_sequence) {
                       return active_sequence &&
                              (*active_sequence == sequence_id);
                     });
}

}  // namespace base
