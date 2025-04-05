#include "base/message_loop/message_loop_impl.h"

#include "base/bind.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/threading/delayed_task_manager_shared_instance.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace base {

namespace {

void RunTask(MessagePump::PendingTask&& pending_task, bool set_scoped_handles) {
  if (pending_task.sequence_id && set_scoped_handles) {
    const auto scoped_sequence_id =
        detail::ScopedSequenceIdSetter{*pending_task.sequence_id};
    const auto scoped_task_runner_handle =
        SequencedTaskRunnerHandle{pending_task.target_task_runner.lock()};

    std::move(pending_task.task).Run();
  } else {
    std::move(pending_task.task).Run();
  }
}

}  // namespace

MessageLoopImpl::MessageLoopImpl(MessagePump::ExecutorId executor_id,
                                 std::shared_ptr<MessagePump> message_pump,
                                 bool set_scoped_handles)
    : set_scoped_handles_(set_scoped_handles),
      executor_id_(executor_id),
      message_pump_(std::move(message_pump)),
      is_stopped_(false) {}

// TODO: maybe should RunUntilIdle() based on input options?
MessageLoopImpl::~MessageLoopImpl() = default;

bool MessageLoopImpl::RunOnce() {
  return DoRunOnce(true);
}

void MessageLoopImpl::RunUntilIdle() {
  while (DoRunOnce(false)) {
  }
}

void MessageLoopImpl::Run() {
  while (!is_stopped_) {
    RunUntilIdleOrStop();
  }
  RunUntilIdle();
}

void MessageLoopImpl::Stop(MessagePump::PendingTask last_task) {
  is_stopped_ = true;
  message_pump_->Stop(std::move(last_task));
}

bool MessageLoopImpl::DoRunOnce(bool wait_for_task) {
  if (auto pending_task =
          message_pump_->GetNextPendingTask(executor_id_, wait_for_task)) {
    RunTask(std::move(pending_task), set_scoped_handles_);
    return true;
  }
  return false;
}

void MessageLoopImpl::RunUntilIdleOrStop() {
  while (!is_stopped_ && DoRunOnce(false)) {
  }
}

}  // namespace base
