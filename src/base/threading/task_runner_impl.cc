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
    std::optional<SequenceId> sequence_id = {},
    const std::optional<MessagePump::ExecutorId>& executor_id = {}) {
  (void)location;

  if (delay.IsZero() || delay.IsNegative()) {
    if (auto pump = weak_pump.lock()) {
      return pump->QueuePendingTask(
          {std::move(task), std::move(sequence_id), executor_id});
    }
  } else {
    delayed_task_manager->QueueDelayedTask(DelayedTaskManager::DelayedTask{
        (TimeTicks::Now() + delay), weak_pump,
        MessagePump::PendingTask{std::move(task), std::move(sequence_id),
                                 executor_id}});
  }

  return false;
}

bool DoRunsInCurrentSequence(const SequenceId& sequence_id) {
  return detail::CurrentSequenceIdHelper::IsCurrentSequence(sequence_id);
}
}  // namespace

TaskRunnerImpl::TaskRunnerImpl(
    std::weak_ptr<MessagePump> pump,
    std::shared_ptr<DelayedTaskManager> delayed_task_manager)
    : pump_(std::move(pump)),
      delayed_task_manager_(std::move(delayed_task_manager)) {
  DCHECK(delayed_task_manager_);
}

bool TaskRunnerImpl::PostDelayedTask(SourceLocation location,
                                     OnceClosure task,
                                     TimeDelta delay) {
  return DoPostTask(std::move(location), std::move(task), std::move(delay),
                    delayed_task_manager_, pump_);
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

bool SequencedTaskRunnerImpl::PostDelayedTask(SourceLocation location,
                                              OnceClosure task,
                                              TimeDelta delay) {
  return DoPostTask(std::move(location), std::move(task), std::move(delay),
                    delayed_task_manager_, pump_, sequence_id_);
}

bool SequencedTaskRunnerImpl::RunsTasksInCurrentSequence() const {
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

bool SingleThreadTaskRunnerImpl::PostDelayedTask(SourceLocation location,
                                                 OnceClosure task,
                                                 TimeDelta delay) {
  return DoPostTask(std::move(location), std::move(task), std::move(delay),
                    delayed_task_manager_, pump_, sequence_id_, executor_id_);
}

bool SingleThreadTaskRunnerImpl::RunsTasksInCurrentSequence() const {
  return DoRunsInCurrentSequence(sequence_id_);
}

}  // namespace base
