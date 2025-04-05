#pragma once

#include "base/message_loop/message_loop.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace base {

class RunLoop {
 public:
  RunLoop();
  ~RunLoop();

  std::shared_ptr<SingleThreadTaskRunner> TaskRunner();

  void RunOnce();
  void RunUntilIdle();

  void Run();
  void Quit();
  void Quit(OnceClosure last_task);
  RepeatingClosure QuitClosure();
  RepeatingCallback<void(OnceClosure)> QuitCallback();

 protected:
  std::unique_ptr<detail::ScopedSequenceIdSetter> scoped_sequence_id_;
  std::unique_ptr<SequencedTaskRunnerHandle> scoped_task_runner_handle_;

  SequenceId sequence_id_;
  std::shared_ptr<SingleThreadTaskRunner> task_runner_;

  std::shared_ptr<MessageLoop> message_loop_;
};

}  // namespace base
