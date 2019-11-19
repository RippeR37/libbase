#pragma once

#include <condition_variable>
#include <list>
#include <mutex>

#include "base/message_loop/message_pump.h"

namespace base {

class MessagePumpImpl : public MessagePump {
 public:
  MessagePumpImpl();

  // MessagePump
  PendingTask GetNextPendingTask(ExecutorId executor_id) override;
  void QueuePendingTask(PendingTask pending_task) override;
  void Stop() override;

 private:
  PendingTask GetNextPendingTask_Locked(ExecutorId executor_id);

  std::mutex mutex_;
  std::condition_variable cond_var_;
  bool stopped_;
  std::list<PendingTask> pending_tasks_;  // TODO: move to separate class
};

}  // namespace base
