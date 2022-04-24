#include "base/threading/task_runner_impl.h"

#include "base/sequenced_task_runner_helpers.h"
#include "base/threading/delayed_task_manager.h"
#include "base/time/time_ticks.h"

namespace base {

namespace {
bool DoPostTask(
    SourceLocation location,
    OnceClosure task,
    TimeDelta delay,
    std::shared_ptr<DelayedTaskManager>& delayed_task_manager,
    const std::weak_ptr<MessagePump>& weak_pump,
    std::weak_ptr<SequencedTaskRunner> target_sequenced_task_runner,
    std::optional<SequenceId> sequence_id = {},
    const std::optional<MessagePump::ExecutorId>& executor_id = {}) {
  (void)location;

  if (delay.IsZero() || delay.IsNegative()) {
    if (auto pump = weak_pump.lock()) {
      return pump->QueuePendingTask({std::move(task), std::move(sequence_id),
                                     executor_id,
                                     std::move(target_sequenced_task_runner)});
    }
  } else {
    delayed_task_manager->QueueDelayedTask(DelayedTaskManager::DelayedTask{
        (TimeTicks::Now() + delay), weak_pump,
        MessagePump::PendingTask{std::move(task), std::move(sequence_id),
                                 executor_id,
                                 std::move(target_sequenced_task_runner)}});
  }

  return false;
}

bool DoRunsInCurrentSequence(const SequenceId& sequence_id) {
  return detail::CurrentSequenceIdHelper::IsCurrentSequence(sequence_id);
}
}  // namespace

//
// TaskRunnerImpl
//

// static
std::shared_ptr<TaskRunnerImpl> TaskRunnerImpl::Create(
    std::weak_ptr<MessagePump> pump,
    std::shared_ptr<DelayedTaskManager> delayed_task_manager) {
  return std::shared_ptr<TaskRunnerImpl>(
      new TaskRunnerImpl(std::move(pump), std::move(delayed_task_manager)));
}

bool TaskRunnerImpl::PostDelayedTask(SourceLocation location,
                                     OnceClosure task,
                                     TimeDelta delay) {
  return DoPostTask(std::move(location), std::move(task), std::move(delay),
                    delayed_task_manager_, pump_, {});
}

TaskRunnerImpl::TaskRunnerImpl(
    std::weak_ptr<MessagePump> pump,
    std::shared_ptr<DelayedTaskManager> delayed_task_manager)
    : pump_(std::move(pump)),
      delayed_task_manager_(std::move(delayed_task_manager)) {
  DCHECK(delayed_task_manager_);
}

//
// SequencedTaskRunnerImpl
//

// static
std::shared_ptr<SequencedTaskRunnerImpl> SequencedTaskRunnerImpl::Create(
    std::weak_ptr<MessagePump> pump,
    SequenceId sequence_id,
    std::shared_ptr<DelayedTaskManager> delayed_task_manager) {
  return std::shared_ptr<SequencedTaskRunnerImpl>(new SequencedTaskRunnerImpl(
      std::move(pump), sequence_id, std::move(delayed_task_manager)));
}

bool SequencedTaskRunnerImpl::PostDelayedTask(SourceLocation location,
                                              OnceClosure task,
                                              TimeDelta delay) {
  return DoPostTask(std::move(location), std::move(task), std::move(delay),
                    delayed_task_manager_, pump_, weak_from_this(),
                    sequence_id_);
}

bool SequencedTaskRunnerImpl::RunsTasksInCurrentSequence() const {
  return DoRunsInCurrentSequence(sequence_id_);
}

SequencedTaskRunnerImpl::SequencedTaskRunnerImpl(
    std::weak_ptr<MessagePump> pump,
    SequenceId sequence_id,
    std::shared_ptr<DelayedTaskManager> delayed_task_manager)
    : pump_(std::move(pump)),
      sequence_id_(std::move(sequence_id)),
      delayed_task_manager_(std::move(delayed_task_manager)) {
  DCHECK(delayed_task_manager_);
}

//
// SingleThreadTaskRunnerImpl
//

// static
std::shared_ptr<SingleThreadTaskRunnerImpl> SingleThreadTaskRunnerImpl::Create(
    std::weak_ptr<MessagePump> pump,
    SequenceId sequence_id,
    MessagePump::ExecutorId executor_id,
    std::shared_ptr<DelayedTaskManager> delayed_task_manager) {
  return std::shared_ptr<SingleThreadTaskRunnerImpl>(
      new SingleThreadTaskRunnerImpl(std::move(pump), sequence_id, executor_id,
                                     std::move(delayed_task_manager)));
}

bool SingleThreadTaskRunnerImpl::PostDelayedTask(SourceLocation location,
                                                 OnceClosure task,
                                                 TimeDelta delay) {
  return DoPostTask(std::move(location), std::move(task), std::move(delay),
                    delayed_task_manager_, pump_, weak_from_this(),
                    sequence_id_, executor_id_);
}

bool SingleThreadTaskRunnerImpl::RunsTasksInCurrentSequence() const {
  return DoRunsInCurrentSequence(sequence_id_);
}

SingleThreadTaskRunnerImpl::SingleThreadTaskRunnerImpl(
    std::weak_ptr<MessagePump> pump,
    SequenceId sequence_id,
    MessagePump::ExecutorId executor_id,
    std::shared_ptr<DelayedTaskManager> delayed_task_manager)
    : pump_(std::move(pump)),
      sequence_id_(std::move(sequence_id)),
      executor_id_(std::move(executor_id)),
      delayed_task_manager_(std::move(delayed_task_manager)) {
  DCHECK(delayed_task_manager_);
}

}  // namespace base
