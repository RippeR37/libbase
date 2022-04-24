#pragma once

#include <optional>

#include "base/callback.h"
#include "base/source_location.h"
#include "base/task_runner_internals.h"
#include "base/time/time_delta.h"

namespace base {

class TaskRunner {
 public:
  virtual ~TaskRunner() = default;

  bool PostTask(SourceLocation location, OnceClosure task);

  virtual bool PostDelayedTask(SourceLocation location,
                               OnceClosure task,
                               TimeDelta delay) = 0;

  bool PostTaskAndReply(SourceLocation location,
                        OnceClosure task,
                        OnceClosure reply);

  template <typename TaskResult,
            typename ReplyArgument,
            template <typename>
            class TaskCallback,
            template <typename>
            class ReplyCallback>
  bool PostTaskAndReplyWithResult(SourceLocation location,
                                  TaskCallback<TaskResult()> task,
                                  ReplyCallback<void(ReplyArgument)> reply) {
    using Helper = detail::PostTaskAndReplyWithResultHelper<
        TaskResult, TaskCallback<TaskResult()>,
        ReplyCallback<void(ReplyArgument)>>;

    auto helper = std::make_unique<Helper>();
    auto* helper_ptr = helper.get();

    return PostTaskAndReply(location,
                            BindOnce(&Helper::ExecuteTaskAndStoreResultIn,
                                     std::move(task), helper_ptr),
                            BindOnce(&Helper::ExecuteReplyWithTaskResult,
                                     std::move(reply), std::move(helper)));
  }
};

}  // namespace base
