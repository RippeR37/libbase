#pragma once

#include <memory>
#include <optional>

#include "base/message_loop/message_pump.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/task_runner.h"

namespace base {

class DelayedTaskManager;

class TaskRunnerImpl : public TaskRunner {
 public:
  static std::shared_ptr<TaskRunnerImpl> Create(
      std::weak_ptr<MessagePump> pump,
      std::shared_ptr<DelayedTaskManager> delayed_task_manager);

  // TaskRunner
  bool PostDelayedTask(SourceLocation location,
                       OnceClosure task,
                       TimeDelta delay) override;

 private:
  explicit TaskRunnerImpl(
      std::weak_ptr<MessagePump> pump,
      std::shared_ptr<DelayedTaskManager> delayed_task_manager);

  std::weak_ptr<MessagePump> pump_;
  std::shared_ptr<DelayedTaskManager> delayed_task_manager_;
};

class SequencedTaskRunnerImpl
    : public SequencedTaskRunner,
      public std::enable_shared_from_this<SequencedTaskRunnerImpl> {
 public:
  static std::shared_ptr<SequencedTaskRunnerImpl> Create(
      std::weak_ptr<MessagePump> pump,
      SequenceId sequence_id,
      std::shared_ptr<DelayedTaskManager> delayed_task_manager);

  // SequencedTaskRunner
  bool PostDelayedTask(SourceLocation location,
                       OnceClosure task,
                       TimeDelta delay) override;
  bool RunsTasksInCurrentSequence() const override;

 private:
  SequencedTaskRunnerImpl(
      std::weak_ptr<MessagePump> pump,
      SequenceId sequence_id,
      std::shared_ptr<DelayedTaskManager> delayed_task_manager);

  std::weak_ptr<MessagePump> pump_;
  SequenceId sequence_id_;
  std::shared_ptr<DelayedTaskManager> delayed_task_manager_;
};

class SingleThreadTaskRunnerImpl
    : public SingleThreadTaskRunner,
      public std::enable_shared_from_this<SingleThreadTaskRunnerImpl> {
 public:
  static std::shared_ptr<SingleThreadTaskRunnerImpl> Create(
      std::weak_ptr<MessagePump> pump,
      SequenceId sequence_id,
      MessagePump::ExecutorId executor_id,
      std::shared_ptr<DelayedTaskManager> delayed_task_manager);

  // SingleThreadTaskRunner
  bool PostDelayedTask(SourceLocation location,
                       OnceClosure task,
                       TimeDelta delay) override;
  bool RunsTasksInCurrentSequence() const override;

 private:
  SingleThreadTaskRunnerImpl(
      std::weak_ptr<MessagePump> pump,
      SequenceId sequence_id,
      MessagePump::ExecutorId executor_id,
      std::shared_ptr<DelayedTaskManager> delayed_task_manager);

  std::weak_ptr<MessagePump> pump_;
  SequenceId sequence_id_;
  MessagePump::ExecutorId executor_id_;
  std::shared_ptr<DelayedTaskManager> delayed_task_manager_;
};

}  // namespace base
