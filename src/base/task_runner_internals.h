#pragma once

#include <memory>
#include <optional>

namespace base {
namespace detail {

template <typename TaskResult, typename TaskCallback, typename ReplyCallback>
class PostTaskAndReplyWithResultHelper {
 public:
  static void ExecuteTaskAndStoreResultIn(
      TaskCallback task,
      PostTaskAndReplyWithResultHelper* helper) {
    helper->result = std::move(task).Run();
  }

  static void ExecuteReplyWithTaskResult(
      ReplyCallback reply,
      std::unique_ptr<PostTaskAndReplyWithResultHelper> helper) {
    std::move(reply).Run(std::move(*helper->result));
  }

 private:
  std::optional<TaskResult> result;
};

}  // namespace detail
}  // namespace base
