#include "base/threading/sequenced_task_runner_handle.h"

#include "base/logging.h"
#include "base/sequenced_task_runner.h"

namespace base {

namespace {
thread_local SequencedTaskRunnerHandle* g_handle = nullptr;
}

// static
const std::shared_ptr<SequencedTaskRunner>& SequencedTaskRunnerHandle::Get() {
  CHECK(g_handle);
  return g_handle->task_runner_;
}

// static
bool SequencedTaskRunnerHandle::IsSet() {
  return g_handle;
}

SequencedTaskRunnerHandle::SequencedTaskRunnerHandle(
    std::shared_ptr<SequencedTaskRunner> task_runner)
    : task_runner_(std::move(task_runner)) {
  DCHECK(task_runner_);
  DCHECK(task_runner_->RunsTasksInCurrentSequence());
  DCHECK(!IsSet());

  g_handle = this;
}

SequencedTaskRunnerHandle::~SequencedTaskRunnerHandle() {
  DCHECK(task_runner_->RunsTasksInCurrentSequence());
  DCHECK_EQ(g_handle, this);

  g_handle = nullptr;
}

}  // namespace base
