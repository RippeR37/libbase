#pragma once

#include <memory>

namespace base {

class SequencedTaskRunner;

class SequencedTaskRunnerHandle {
 public:
  // Can be called only if IsSet() returns true.
  static const std::shared_ptr<SequencedTaskRunner>& Get();
  static bool IsSet();

  explicit SequencedTaskRunnerHandle(
      std::shared_ptr<SequencedTaskRunner> task_runner);
  ~SequencedTaskRunnerHandle();

  SequencedTaskRunnerHandle(const SequencedTaskRunnerHandle&) = delete;
  SequencedTaskRunnerHandle& operator=(const SequencedTaskRunnerHandle&) =
      delete;

 private:
  std::shared_ptr<SequencedTaskRunner> task_runner_;
};

}  // namespace base
