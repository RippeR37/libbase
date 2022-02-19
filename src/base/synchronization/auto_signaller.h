#pragma once

#include <cstddef>

namespace base {

class WaitableEvent;

class AutoSignaller {
 public:
  explicit AutoSignaller(WaitableEvent* event);
  explicit AutoSignaller(std::nullptr_t) = delete;
  ~AutoSignaller();

  AutoSignaller(AutoSignaller&& other);
  AutoSignaller& operator=(AutoSignaller&& other);

  void SignalAndReset();
  void Cancel();

 private:
  WaitableEvent* event_;
};

}  // namespace base
