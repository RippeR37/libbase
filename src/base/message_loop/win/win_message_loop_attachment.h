#pragma once

#if defined(LIBBASE_IS_WINDOWS)

#include <memory>

#include "base/message_loop/message_loop.h"
#include "base/platform/windows.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/delayed_task_manager_shared_instance.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace base {
namespace win {

class WinMessageLoopAttachment {
 public:
  static std::unique_ptr<WinMessageLoopAttachment> TryCreate();

 public:
  ~WinMessageLoopAttachment();

  std::shared_ptr<SingleThreadTaskRunner> TaskRunner() const;

 private:
  WinMessageLoopAttachment(HWND hWnd);

  static LRESULT CALLBACK WindowProc(HWND hWnd,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam);

  HWND hWnd_;
  SequenceId sequence_id_;
  std::shared_ptr<MessagePump> message_pump_;
  std::shared_ptr<SingleThreadTaskRunner> task_runner_;
  base::detail::ScopedSequenceIdSetter scoped_sequence_id_;
  SequencedTaskRunnerHandle scoped_task_runner_handle_;
};

}  // namespace win
}  // namespace base

#endif  // defined(LIBBASE_IS_WINDOWS)
