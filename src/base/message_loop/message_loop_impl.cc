#include "base/message_loop/message_loop_impl.h"

#include "base/bind.h"
#include "base/sequenced_task_runner_helpers.h"

namespace base {

MessageLoopImpl::MessageLoopImpl(MessagePump::ExecutorId executor_id,
                                 std::shared_ptr<MessagePump> message_pump)
    : executor_id_(executor_id),
      message_pump_(std::move(message_pump)),
      is_stopped_(false) {}

MessageLoopImpl::~MessageLoopImpl() {
  // TODO: maybe should RunUntilIdle() based on input options?
}

bool MessageLoopImpl::RunOnce() {
  if (auto pending_task = message_pump_->GetNextPendingTask(executor_id_)) {
    RunTask(std::move(pending_task));
    return true;
  }
  return false;
}

void MessageLoopImpl::RunUntilIdle() {
  while (RunOnce()) {
  }
}

void MessageLoopImpl::Run() {
  while (!is_stopped_) {
    RunUntilIdleOrStop();
  }
}

void MessageLoopImpl::Stop() {
  is_stopped_ = true;
  message_pump_->Stop();
}

void MessageLoopImpl::RunUntilIdleOrStop() {
  while (!is_stopped_ && RunOnce()) {
  }
}

void MessageLoopImpl::RunTask(MessagePump::PendingTask&& pending_task) const {
  if (pending_task.sequence_id) {
    const auto scoped_sequence_id =
        detail::ScopedSequenceIdSetter{*pending_task.sequence_id};
    std::move(pending_task.task).Run();
  } else {
    std::move(pending_task.task).Run();
  }
}

}  // namespace base
