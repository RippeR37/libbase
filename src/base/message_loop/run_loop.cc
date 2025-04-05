#include "base/message_loop/run_loop.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop_impl.h"
#include "base/message_loop/message_pump_impl.h"
#include "base/threading/delayed_task_manager_shared_instance.h"
#include "base/threading/task_runner_impl.h"

namespace base {

namespace {
const size_t kMainThreadExecutorCount = 1;
const auto kMainThreadExecutorId = 0;
}  // namespace

RunLoop::RunLoop()
    : sequence_id_(detail::SequenceIdGenerator::GetNextSequenceId()) {
  auto message_pump =
      std::make_shared<MessagePumpImpl>(kMainThreadExecutorCount);
  message_loop_ = std::make_shared<MessageLoopImpl>(kMainThreadExecutorId,
                                                    message_pump, false);

  std::weak_ptr<MessagePump> weak_message_pump = message_pump;
  task_runner_ = SingleThreadTaskRunnerImpl::Create(
      weak_message_pump, sequence_id_, kMainThreadExecutorId,
      DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance());

  scoped_sequence_id_ =
      std::make_unique<detail::ScopedSequenceIdSetter>(sequence_id_);
  scoped_task_runner_handle_ =
      std::make_unique<SequencedTaskRunnerHandle>(task_runner_);
}

RunLoop::~RunLoop() {
  Quit();
}

std::shared_ptr<SingleThreadTaskRunner> RunLoop::TaskRunner() {
  return task_runner_;
}

void RunLoop::RunOnce() {
  message_loop_->RunOnce();
}

void RunLoop::RunUntilIdle() {
  message_loop_->RunUntilIdle();
}

void RunLoop::Run() {
  message_loop_->Run();
}

void RunLoop::Quit() {
  Quit(OnceClosure{});
}

void RunLoop::Quit(base::OnceClosure last_task) {
  message_loop_->Stop(MessagePump::PendingTask{
      std::move(last_task), sequence_id_, kMainThreadExecutorId, task_runner_});
}

RepeatingClosure RunLoop::QuitClosure() {
  return base::BindRepeating(
      [](std::shared_ptr<MessageLoop> message_loop,
         std::optional<SequenceId> sequence_id,
         std::optional<MessagePump::ExecutorId> allowed_executor_id,
         std::weak_ptr<SequencedTaskRunner> target_task_runner) {
        message_loop->Stop(MessagePump::PendingTask{OnceClosure{}, sequence_id,
                                                    allowed_executor_id,
                                                    target_task_runner});
      },
      message_loop_, sequence_id_, kMainThreadExecutorId, task_runner_);
}

base::RepeatingCallback<void(base::OnceClosure)> RunLoop::QuitCallback() {
  return base::BindRepeating(
      [](std::shared_ptr<MessageLoop> message_loop,
         std::optional<SequenceId> sequence_id,
         std::optional<MessagePump::ExecutorId> allowed_executor_id,
         std::weak_ptr<SequencedTaskRunner> target_task_runner,
         OnceClosure task) {
        message_loop->Stop(
            MessagePump::PendingTask{std::move(task), sequence_id,
                                     allowed_executor_id, target_task_runner});
      },
      message_loop_, sequence_id_, kMainThreadExecutorId, task_runner_);
}

}  // namespace base
