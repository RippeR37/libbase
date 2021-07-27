#include "base/synchronization/waitable_event.h"

namespace base {

WaitableEvent::WaitableEvent(ResetPolicy reset_policy,
                             InitialState initial_state)
    : reset_policy_(reset_policy),
      is_signaled_(initial_state == InitialState::kSignaled) {}

WaitableEvent::~WaitableEvent() = default;

void WaitableEvent::Reset() {
  std::lock_guard<std::mutex> guard{mutex_};
  ResetLocked();
}

void WaitableEvent::Signal() {
  {
    std::lock_guard<std::mutex> guard{mutex_};
    is_signaled_ = true;
  }
  cond_var_.notify_one();
}

bool WaitableEvent::IsSignaled() {
  std::lock_guard<std::mutex> guard{mutex_};
  return IsSignaledLocked();
}

void WaitableEvent::Wait() {
  std::unique_lock<std::mutex> guard{mutex_};

  if (IsSignaledLocked()) {
    return;
  }

  cond_var_.wait(guard, [&]() { return IsSignaledLocked(); });
}

void WaitableEvent::ResetLocked() {
  is_signaled_ = false;
}

bool WaitableEvent::IsSignaledLocked() {
  const auto is_signaled = is_signaled_;
  if (is_signaled && reset_policy_ == ResetPolicy::kAutomatic) {
    ResetLocked();
  }
  return is_signaled;
}

}  // namespace base
