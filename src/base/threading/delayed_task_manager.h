#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "base/message_loop/message_pump.h"
#include "base/time/time_ticks.h"

namespace base {

class DelayedTaskManager {
 public:
  struct DelayedTask {
    bool operator<(const DelayedTask& rhs) const;

    TimeTicks start_time;
    std::weak_ptr<MessagePump> message_pump;
    mutable MessagePump::PendingTask pending_task;
  };

  DelayedTaskManager();
  ~DelayedTaskManager();

  void QueueDelayedTask(DelayedTask delayed_task);

 private:
  void ScheduleTasksUntilStop();
  void ScheduleAllReadyTasksLocked();
  void WaitForNextTaskOrStopLocked(std::unique_lock<std::mutex>& lock);
  std::optional<TimeDelta> NextTaskRemainingDelayLocked() const;

  std::thread scheduler_thread_;
  std::mutex mutex_;
  std::condition_variable cond_var_;

  // Everything below is locked behind |mutex_|.
  bool stopped_;
  std::priority_queue<DelayedTask> delayed_tasks_;
};

}  // namespace base
