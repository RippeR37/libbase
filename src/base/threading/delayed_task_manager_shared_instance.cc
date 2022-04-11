#include "base/threading/delayed_task_manager_shared_instance.h"

#include "base/threading/delayed_task_manager.h"

namespace base {

// static
std::shared_ptr<DelayedTaskManager>
DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance() {
  auto& instance = GetInstance();

  std::lock_guard<std::mutex> guard{instance.mutex_};
  if (auto x = instance.current_manager_.lock()) {
    return x;
  }
  std::shared_ptr<DelayedTaskManager> new_manager{new DelayedTaskManager()};
  instance.current_manager_ = new_manager;
  return new_manager;
}

// static
DelayedTaskManagerSharedInstance&
DelayedTaskManagerSharedInstance::GetInstance() {
  static DelayedTaskManagerSharedInstance instance;
  return instance;
}

}  // namespace base
