#include "base/synchronization/auto_signaller.h"

#include "base/synchronization/waitable_event.h"

namespace base {

AutoSignaller::AutoSignaller(WaitableEvent* event) : event_(event) {}

AutoSignaller::~AutoSignaller() {
  if (event_) {
    event_->Signal();
  }
}

AutoSignaller::AutoSignaller(AutoSignaller&& other)
    : event_(std::exchange(other.event_, nullptr)) {}

AutoSignaller& AutoSignaller::operator=(AutoSignaller&& other) {
  if (&other != this) {
    if (event_) {
      event_->Signal();
    }
    event_ = std::exchange(other.event_, nullptr);
  }
  return *this;
}

void AutoSignaller::SignalAndReset() {
  if (event_) {
    event_->Signal();
    event_ = nullptr;
  }
}

void AutoSignaller::Cancel() {
  event_ = nullptr;
}

}  // namespace base
