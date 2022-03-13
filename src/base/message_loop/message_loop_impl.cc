#include "base/message_loop/message_loop_impl.h"

#include "base/bind.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/threading/task_runner_impl.h"

namespace base {

MessageLoopImpl::MessageLoopImpl(MessagePump::ExecutorId executor_id,
                                 std::shared_ptr<MessagePump> message_pump)
    : executor_id_(executor_id),
      message_pump_(std::move(message_pump)),
      is_stopped_(false) {}

// TODO: maybe should RunUntilIdle() based on input options?
MessageLoopImpl::~MessageLoopImpl() = default;

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
  RunUntilIdle();
}

void MessageLoopImpl::Stop(OnceClosure last_task) {
  is_stopped_ = true;
  message_pump_->Stop(MessagePump::PendingTask{std::move(last_task),
                                               std::nullopt, std::nullopt});
}

void MessageLoopImpl::RunUntilIdleOrStop() {
  while (!is_stopped_ && RunOnce()) {
  }
}

void MessageLoopImpl::RunTask(MessagePump::PendingTask&& pending_task) {
  if (pending_task.sequence_id) {
    const auto scoped_sequence_id =
        detail::ScopedSequenceIdSetter{*pending_task.sequence_id};

    const auto scoped_task_runner_handle =
        SequencedTaskRunnerHandle{std::make_shared<SequencedTaskRunnerImpl>(
            std::weak_ptr<MessagePump>(message_pump_),
            *pending_task.sequence_id)};

    std::move(pending_task.task).Run();
  } else {
    std::move(pending_task.task).Run();
  }
}

}  // namespace base
