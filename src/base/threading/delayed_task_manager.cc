#include "base/threading/delayed_task_manager.h"

#include <chrono>

#include "base/logging.h"

namespace base {

namespace {
void ScheduleTask(const DelayedTaskManager::DelayedTask& delayed_task) {
  if (auto message_pump = delayed_task.message_pump.lock()) {
    message_pump->QueuePendingTask(std::move(delayed_task.pending_task));
  }
}
}  // namespace

bool DelayedTaskManager::DelayedTask::operator<(const DelayedTask& rhs) const {
  // We're returning bigger value, because we want to sort the tasks from the
  // one with smaller start_time at the top, while std::priority_queue<T> - by
  // default - chooses the one with the highest value as top(). This inverses
  // the ordering.
  return start_time > rhs.start_time;
}

DelayedTaskManager::DelayedTaskManager(TimeTicksProvider time_ticks_provider)
    : time_ticks_provider_(time_ticks_provider), stopped_(false) {
  scheduler_thread_ =
      std::thread{&DelayedTaskManager::ScheduleTasksUntilStop, this};
}

DelayedTaskManager::~DelayedTaskManager() {
  {
    std::lock_guard<std::mutex> guard{mutex_};
    stopped_ = true;
  }

  cond_var_.notify_one();
  scheduler_thread_.join();
}

void DelayedTaskManager::QueueDelayedTask(DelayedTask delayed_task) {
  std::lock_guard<std::mutex> lock{mutex_};

  if (stopped_) {
    // Skip scheduling new tasks when stopped.
    return;
  }

  // If can execute right away, do it.
  if (delayed_task.start_time < time_ticks_provider_()) {
    ScheduleTask(delayed_task);
    return;
  }

  // If the new task is the first one or it has smaller start time then the
  // first one then we will need to wake scheduler thread to update how long it
  // is supposed to wait for the first task (or possibly schedule it right
  // away).
  const bool need_to_wake_scheduler =
      delayed_tasks_.empty() ||
      (delayed_task.start_time < delayed_tasks_.top().start_time);

  delayed_tasks_.push(std::move(delayed_task));

  if (need_to_wake_scheduler) {
    cond_var_.notify_one();
  }

  // Try to schedule any pending tasks while we're here.
  ScheduleAllReadyTasksLocked();
}

void DelayedTaskManager::ScheduleAllReadyTasksForTests() {
  std::unique_lock lock(mutex_);
  ScheduleAllReadyTasksLocked();
}

void DelayedTaskManager::ScheduleTasksUntilStop() {
  std::unique_lock lock(mutex_);

  while (!stopped_) {
    ScheduleAllReadyTasksLocked();
    WaitForNextTaskOrStopLocked(lock);
  }
}

void DelayedTaskManager::ScheduleAllReadyTasksLocked() {
  const auto can_run_first_task = [&]() {
    return !delayed_tasks_.empty() &&
           delayed_tasks_.top().start_time <= time_ticks_provider_();
  };

  while (can_run_first_task()) {
    ScheduleTask(delayed_tasks_.top());
    delayed_tasks_.pop();
  }
}

void DelayedTaskManager::WaitForNextTaskOrStopLocked(
    std::unique_lock<std::mutex>& lock) {
  const auto previous_task_count = delayed_tasks_.size();
  const auto can_resume_from_wait = [&]() {
    return stopped_ || previous_task_count != delayed_tasks_.size();
  };

  if (auto next_task_delay = NextTaskRemainingDelayLocked()) {
    cond_var_.wait_for(
        lock, std::chrono::microseconds(next_task_delay->InMicroseconds()),
        can_resume_from_wait);
  } else {
    cond_var_.wait(lock, can_resume_from_wait);
  }
}

std::optional<TimeDelta> DelayedTaskManager::NextTaskRemainingDelayLocked()
    const {
  if (delayed_tasks_.empty()) {
    return {};
  }
  return delayed_tasks_.top().start_time - time_ticks_provider_();
}

}  // namespace base
