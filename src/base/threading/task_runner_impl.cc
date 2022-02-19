#include "base/threading/task_runner_impl.h"

#include "base/sequenced_task_runner_helpers.h"

namespace base {

namespace {
bool DoPostTask(
    SourceLocation location,
    OnceClosure task,
    const std::weak_ptr<MessagePump>& weak_pump,
    std::optional<SequenceId> sequence_id = {},
    const std::optional<MessagePump::ExecutorId>& executor_id = {}) {
  (void)location;

  if (auto pump = weak_pump.lock()) {
    return pump->QueuePendingTask(
        {std::move(task), std::move(sequence_id), executor_id});
  }

  return false;
}

bool DoRunsInCurrentSequence(const SequenceId& sequence_id) {
  return detail::CurrentSequenceIdHelper::IsCurrentSequence(sequence_id);
}
}  // namespace

TaskRunnerImpl::TaskRunnerImpl(std::weak_ptr<MessagePump> pump)
    : pump_(std::move(pump)) {}

bool TaskRunnerImpl::PostTask(SourceLocation location, OnceClosure task) {
  return DoPostTask(std::move(location), std::move(task), pump_);
}

SequencedTaskRunnerImpl::SequencedTaskRunnerImpl(
    std::weak_ptr<MessagePump> pump,
    SequenceId sequence_id)
    : pump_(std::move(pump)), sequence_id_(std::move(sequence_id)) {}

bool SequencedTaskRunnerImpl::PostTask(SourceLocation location,
                                       OnceClosure task) {
  return DoPostTask(std::move(location), std::move(task), pump_, sequence_id_);
}

bool SequencedTaskRunnerImpl::RunsTasksInCurrentSequence() const {
  return DoRunsInCurrentSequence(sequence_id_);
}

SingleThreadTaskRunnerImpl::SingleThreadTaskRunnerImpl(
    std::weak_ptr<MessagePump> pump,
    SequenceId sequence_id,
    MessagePump::ExecutorId executor_id)
    : pump_(std::move(pump)),
      sequence_id_(std::move(sequence_id)),
      executor_id_(std::move(executor_id)) {}

bool SingleThreadTaskRunnerImpl::PostTask(SourceLocation location,
                                          OnceClosure task) {
  return DoPostTask(std::move(location), std::move(task), pump_, sequence_id_,
                    executor_id_);
}

bool SingleThreadTaskRunnerImpl::RunsTasksInCurrentSequence() const {
  return DoRunsInCurrentSequence(sequence_id_);
}

}  // namespace base
