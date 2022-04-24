#pragma once

#include <memory>
#include <mutex>

namespace base {

class DelayedTaskManager;

class DelayedTaskManagerSharedInstance {
 public:
  static std::shared_ptr<DelayedTaskManager> GetOrCreateSharedInstance();

 private:
  static DelayedTaskManagerSharedInstance& GetInstance();

  std::mutex mutex_;
  std::weak_ptr<DelayedTaskManager> current_manager_;
};

}  // namespace base
