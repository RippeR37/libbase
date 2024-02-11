#pragma once

#ifdef LIBBASE_IS_WINDOWS

#include <windows.h>

#include "base/message_loop/win/message_pump_win_impl.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/threading/delayed_task_manager_shared_instance.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/threading/task_runner_impl.h"

namespace base {

template <UINT WM_LIBBASE_EXECUTE_TASK>
class WinThreadAttachment {
 public:
  static LRESULT CALLBACK WindowProc(HWND hWnd,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam) {
    DCHECK_NE(WinThreadAttachment::current_instance_, nullptr);

    switch (uMsg) {
      case WM_LIBBASE_EXECUTE_TASK: {
        const MessagePump::ExecutorId executor_id = 0;
        if (auto pending_task =
                WinThreadAttachment::current_instance_->message_pump_
                    ->GetNextPendingTask(executor_id)) {
          std::move(pending_task.task).Run();
        }
      }
        return DefWindowProc(hWnd, uMsg, wParam, lParam);

      default:
        return WinThreadAttachment::current_instance_->wndProc_(hWnd, uMsg,
                                                                wParam, lParam);
    }
  }

  WinThreadAttachment(HWND hWnd)
      : hWnd_(hWnd),
        wndProc_((WNDPROC)GetWindowLongPtr(hWnd_, GWLP_WNDPROC)),
        sequence_id_(detail::SequenceIdGenerator::GetNextSequenceId()),
        message_pump_(
            std::make_shared<MessagePumpWinImpl<WM_LIBBASE_EXECUTE_TASK>>(
                1,
                hWnd_)),
        task_runner_(SingleThreadTaskRunnerImpl::Create(
            message_pump_,
            sequence_id_,
            0,
            DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance())),
        scoped_sequence_id_(sequence_id_),
        scoped_task_runner_handle_(task_runner_) {
    DCHECK_EQ(WinThreadAttachment::current_instance_, nullptr);
    WinThreadAttachment::current_instance_ = this;

    SetWindowLongPtr(hWnd_, GWLP_WNDPROC,
                     (LONG_PTR)WinThreadAttachment::WindowProc);
  }

  ~WinThreadAttachment() {
    DCHECK_NE(WinThreadAttachment::current_instance_, nullptr);
    WinThreadAttachment::current_instance_ = nullptr;

    message_pump_->Stop({});
    SetWindowLongPtr(hWnd_, GWLP_WNDPROC, (LONG_PTR)wndProc_);
  }

  std::shared_ptr<SingleThreadTaskRunner> TaskRunner() const {
    return task_runner_;
  }

 private:
  inline static thread_local WinThreadAttachment* current_instance_ = nullptr;

  HWND hWnd_;
  WNDPROC wndProc_;

  SequenceId sequence_id_;
  std::shared_ptr<MessagePump> message_pump_;
  std::shared_ptr<SingleThreadTaskRunnerImpl> task_runner_;
  detail::ScopedSequenceIdSetter scoped_sequence_id_;
  SequencedTaskRunnerHandle scoped_task_runner_handle_;
};

}  // namespace base

#endif  // LIBBASE_IS_WINDOWS
