#pragma once

#include <atomic>
#include <memory>

#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump.h"

namespace base {

class MessageLoopImpl : public MessageLoop {
 public:
  MessageLoopImpl(MessagePump::ExecutorId executor_id,
                  std::shared_ptr<MessagePump> message_pump,
                  bool set_scoped_handles = true);
  ~MessageLoopImpl() override;

  // MessageLoop
  bool RunOnce() override;
  void RunUntilIdle() override;
  void Run() override;
  void Stop(MessagePump::PendingTask last_task) override;

 private:
  bool DoRunOnce(bool wait_for_task);
  void RunUntilIdleOrStop();

  const bool set_scoped_handles_;
  const MessagePump::ExecutorId executor_id_;
  std::shared_ptr<MessagePump> message_pump_;
  std::atomic_bool is_stopped_;
};

}  // namespace base
