#include "base/task_runner.h"

#include "base/bind.h"
#include "base/bind_post_task.h"
#include "base/logging.h"
#include "base/sequenced_task_runner.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace base {

namespace {

#ifdef LIBBASE_POLICY_LEAK_ON_REPLY_POST_TASK_FAILURE
class PostTaskAndReplyHelper {
 public:
  static void ExecuteTaskAndPostReply(PostTaskAndReplyHelper helper) {
    std::move(helper.task_).Run();

    const auto original_task_runner = helper.original_task_runner_;
    const auto location = helper.location_;

    original_task_runner->PostTask(
        location,
        BindOnce(&PostTaskAndReplyHelper::ExecuteReply, std::move(helper)));
  }

  static void ExecuteReply(PostTaskAndReplyHelper helper) {
    std::move(helper.reply_).Run();
  }

  PostTaskAndReplyHelper(
      SourceLocation location,
      OnceClosure task,
      OnceClosure reply,
      std::shared_ptr<SequencedTaskRunner> original_task_runner)
      : location_(std::move(location)),
        task_(std::move(task)),
        reply_(std::move(reply)),
        original_task_runner_(std::move(original_task_runner)) {}

  ~PostTaskAndReplyHelper() {
    if (reply_ && original_task_runner_ &&
        !original_task_runner_->RunsTasksInCurrentSequence()) {
      // Leak reply callback to avoid hitting (D)CHECKs for sequence affinity
      (void)std::make_unique<OnceClosure>(std::move(reply_)).release();
    }
  }

  PostTaskAndReplyHelper(PostTaskAndReplyHelper&&) = default;
  PostTaskAndReplyHelper& operator=(PostTaskAndReplyHelper&&) = default;

 private:
  SourceLocation location_;
  OnceClosure task_;
  OnceClosure reply_;
  std::shared_ptr<SequencedTaskRunner> original_task_runner_;
};
#endif  // LIBBASE_POLICY_LEAK_ON_REPLY_POST_TASK_FAILURE

}  // namespace

bool TaskRunner::PostTaskAndReply(SourceLocation location,
                                  OnceClosure task,
                                  OnceClosure reply) {
  DCHECK(task);
  DCHECK(reply);
  DCHECK(SequencedTaskRunnerHandle::IsSet());

#ifdef LIBBASE_POLICY_LEAK_ON_REPLY_POST_TASK_FAILURE
  return PostTask(location,
                  BindOnce(&PostTaskAndReplyHelper::ExecuteTaskAndPostReply,
                           PostTaskAndReplyHelper{
                               location, std::move(task), std::move(reply),
                               SequencedTaskRunnerHandle::Get()}));
#else   // LIBBASE_POLICY_LEAK_ON_REPLY_POST_TASK_FAILURE
  return PostTask(location, std::move(task).Then(base::BindPostTask(
                                SequencedTaskRunnerHandle::Get(),
                                std::move(reply), location)));
#endif  // LIBBASE_POLICY_LEAK_ON_REPLY_POST_TASK_FAILURE
}

}  // namespace base
