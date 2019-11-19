#include "base/message_loop/message_pump_impl.h"

namespace base {

MessagePumpImpl::MessagePumpImpl() : stopped_(false) {}

MessagePumpImpl::PendingTask MessagePumpImpl::GetNextPendingTask(
    ExecutorId executor_id) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (stopped_) {
    return {};
  }

  if (auto pending_task = GetNextPendingTask_Locked(executor_id)) {
    return pending_task;
  }

  cond_var_.wait(lock, [&]() { return (!pending_tasks_.empty() || stopped_); });
  return GetNextPendingTask_Locked(executor_id);
}

void MessagePumpImpl::QueuePendingTask(PendingTask pending_task) {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    if (!stopped_) {
      pending_tasks_.push_back(std::move(pending_task));
    }
  }

  cond_var_.notify_one();
}

void MessagePumpImpl::Stop() {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    stopped_ = true;
  }

  cond_var_.notify_all();
}

MessagePumpImpl::PendingTask MessagePumpImpl::GetNextPendingTask_Locked(
    ExecutorId executor_id) {
  if (stopped_) {
    return {};
  }

  const auto is_pending_task_allowed = [executor_id](const PendingTask& task) {
    return (task.allowed_executor_id.value_or(executor_id) == executor_id);
  };

  const auto allowed_pending_task_iter = std::find_if(
      pending_tasks_.begin(), pending_tasks_.end(), is_pending_task_allowed);

  if (allowed_pending_task_iter == pending_tasks_.end()) {
    return {};
  }

  PendingTask task = std::move(*allowed_pending_task_iter);
  pending_tasks_.erase(allowed_pending_task_iter);

  return task;
}

}  // namespace base
