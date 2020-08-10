#pragma once

#include <condition_variable>
#include <mutex>

namespace base {

class WaitableEvent {
 public:
  // Automatic reset policy means that the event state will reset each time
  // after a single thread stops waiting for event signalling. In manual mode,
  // state will reset after a call to Reset().
  enum class ResetPolicy {
    kManual,
    kAutomatic,
  };

  // Specifies whether event should be initally in a signaled state or not.
  enum class InitialState {
    kSignaled,
    kNotSignaled,
  };

  WaitableEvent(ResetPolicy reset_policy = ResetPolicy::kManual,
                InitialState initial_state = InitialState::kNotSignaled);
  ~WaitableEvent();

  // Resets event's state to not-signaled state.
  void Reset();

  // Changes event's state to signaled which may wake a thread blocked on Wait()
  // call.
  void Signal();

  // Returns true if the event is in signaled state, false otherwise.
  // For event with automatic reset policy, this will reset the state to not
  // signaled.
  bool IsSignaled();

  // Wait indefinitely for the event to be signaled.
  void Wait();

  // TODO: implement timed waits
  // TODO: implement WaitMany()

 private:
  void ResetLocked();
  bool IsSignaledLocked();

  const ResetPolicy reset_policy_;
  bool is_signaled_;
  std::mutex mutex_;
  std::condition_variable cond_var_;
};

}  // namespace base
