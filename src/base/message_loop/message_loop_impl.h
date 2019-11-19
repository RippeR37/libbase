#pragma once

#include <atomic>
#include <memory>

#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump.h"

namespace base {

class MessagePumpImpl;

class MessageLoopImpl : public MessageLoop {
 public:
  MessageLoopImpl(MessagePump::ExecutorId executor_id,
                  std::shared_ptr<MessagePumpImpl> pump);
  ~MessageLoopImpl() override;

  bool RunOnce() override;
  void RunUntilIdle() override;

  void Run() override;
  void Stop() override;

 private:
  void RunUntilIdleOrStop();

  void RunTask(MessagePump::PendingTask&& pending_task) const;

  const MessagePump::ExecutorId executor_id_;
  std::shared_ptr<MessagePumpImpl> pump_;
  std::atomic_bool is_stopped_;
};

}  // namespace base
