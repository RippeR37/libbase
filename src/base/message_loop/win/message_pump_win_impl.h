#pragma once

#ifdef LIBBASE_IS_WINDOWS

#include <windows.h>

#include "base/message_loop/message_pump_impl.h"

namespace base {

template <UINT WM_LIBBASE_EXECUTE_TASK>
class MessagePumpWinImpl : public MessagePumpImpl {
 public:
  MessagePumpWinImpl(size_t executors_count, HWND hwnd)
      : MessagePumpImpl(executors_count), hwnd_(hwnd) {}

  // MessagePump
  bool QueuePendingTask(PendingTask pending_task) override {
    if (MessagePumpImpl::QueuePendingTask(std::move(pending_task))) {
      PostMessage(hwnd_, WM_LIBBASE_EXECUTE_TASK, 0, 0);
      return true;
    }
    return false;
  }

 private:
  HWND hwnd_;
};

}  // namespace base

#endif  // LIBBASE_IS_WINDOWS
