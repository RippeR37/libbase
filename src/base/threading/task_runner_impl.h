#pragma once

#include <memory>
#include <optional>

#include "base/message_loop/message_pump.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/task_runner.h"

namespace base {

class TaskRunnerImpl : public TaskRunner {
 public:
  TaskRunnerImpl(std::weak_ptr<MessagePump> pump);

  // TaskRunner
  bool PostTask(SourceLocation location, OnceClosure task) override;

 private:
  std::weak_ptr<MessagePump> pump_;
};

class SequencedTaskRunnerImpl : public SequencedTaskRunner {
 public:
  SequencedTaskRunnerImpl(std::weak_ptr<MessagePump> pump,
                          SequenceId sequence_id);

  // SequencedTaskRunner
  bool PostTask(SourceLocation location, OnceClosure task) override;
  bool RunsTasksInCurrentSequence() const override;

 private:
  std::weak_ptr<MessagePump> pump_;
  SequenceId sequence_id_;
};

class SingleThreadTaskRunnerImpl : public SingleThreadTaskRunner {
 public:
  SingleThreadTaskRunnerImpl(std::weak_ptr<MessagePump> pump,
                             SequenceId sequence_id,
                             MessagePump::ExecutorId executor_id);

  // SingleThreadTaskRunner
  bool PostTask(SourceLocation location, OnceClosure task) override;
  bool RunsTasksInCurrentSequence() const override;

 private:
  std::weak_ptr<MessagePump> pump_;
  SequenceId sequence_id_;
  MessagePump::ExecutorId executor_id_;
};

}  // namespace base
