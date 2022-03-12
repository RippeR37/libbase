#include "base/barrier_closure.h"

#include <atomic>

namespace base {

namespace {
class BarrierClosureHelper {
 public:
  BarrierClosureHelper(size_t required_run_count, OnceClosure callback)
      : required_run_count_(static_cast<int>(required_run_count)),
        callback_(std::move(callback)) {}

  void Run() {
    const auto runs_left = --required_run_count_;
    DCHECK_GE(runs_left, 0);
    if (runs_left == 0) {
      std::move(callback_).Run();
    }
  }

 private:
  std::atomic_int required_run_count_;
  OnceClosure callback_;
};
}  // namespace

RepeatingClosure BarrierClosure(size_t required_run_count,
                                OnceClosure callback) {
  if (required_run_count == 0) {
    std::move(callback).Run();
    return {};
  }

  return BindRepeating(&BarrierClosureHelper::Run,
                       Owned(std::make_unique<BarrierClosureHelper>(
                           required_run_count, std::move(callback))));
}

}  // namespace base
