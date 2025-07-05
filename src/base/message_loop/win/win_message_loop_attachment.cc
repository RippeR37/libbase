#ifndef UNICODE
#define UNICODE
#endif

#include "base/message_loop/win/win_message_loop_attachment.h"

#include "base/logging.h"
#include "base/message_loop/message_pump_impl.h"
#include "base/threading/task_runner_impl.h"

namespace base {
namespace win {

namespace {
inline const UINT WM_LIBBASE_EXECUTE_TASK = WM_USER + 0;
inline const wchar_t LIBBASE_CLASS_NAME[] = L"libbaseMessageOnlyWindow";

thread_local WinMessageLoopAttachment* g_current_instance = nullptr;

HWND CreateMessageOnlyWindow(WNDPROC wndProc) {
  static int static_in_this_module = 0;

  HMODULE hModule = nullptr;
  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                    reinterpret_cast<LPCWSTR>(&static_in_this_module),
                    &hModule);

  WNDCLASS window_class = {};
  window_class.lpfnWndProc = wndProc;
  window_class.lpszClassName = LIBBASE_CLASS_NAME;
  window_class.hInstance = nullptr;  // hModule;
  window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
  RegisterClass(&window_class);

  return CreateWindow(LIBBASE_CLASS_NAME, L"", 0, CW_USEDEFAULT, CW_USEDEFAULT,
                      CW_USEDEFAULT, CW_USEDEFAULT, HWND_MESSAGE, nullptr,
                      hModule, nullptr);
}
}  // namespace

//
// WinMessagePumpImpl
//

namespace detail {
class WinMessagePumpImpl : public MessagePumpImpl {
 public:
  WinMessagePumpImpl(size_t executors_count, HWND hwnd)
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
}  // namespace detail

//
// WinMessageLoopAttachment
//

// static
std::unique_ptr<WinMessageLoopAttachment>
WinMessageLoopAttachment::TryCreate() {
  if (auto* hWnd =
          CreateMessageOnlyWindow(&WinMessageLoopAttachment::WindowProc)) {
    return std::unique_ptr<WinMessageLoopAttachment>(
        new WinMessageLoopAttachment(hWnd));
  }

  return {};
}

WinMessageLoopAttachment::WinMessageLoopAttachment(HWND hWnd)
    : hWnd_(hWnd),
      sequence_id_(base::detail::SequenceIdGenerator::GetNextSequenceId()),
      message_pump_(std::make_shared<detail::WinMessagePumpImpl>(1, hWnd_)),
      task_runner_(SingleThreadTaskRunnerImpl::Create(
          message_pump_,
          sequence_id_,
          0,
          DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance())),
      scoped_sequence_id_(sequence_id_),
      scoped_task_runner_handle_(task_runner_) {
  DCHECK_EQ(g_current_instance, nullptr);
  g_current_instance = this;
}

WinMessageLoopAttachment::~WinMessageLoopAttachment() {
  DCHECK_NE(g_current_instance, nullptr);
  g_current_instance = nullptr;

  message_pump_->Stop({});
  DestroyWindow(hWnd_);
}

std::shared_ptr<SingleThreadTaskRunner> WinMessageLoopAttachment::TaskRunner()
    const {
  return task_runner_;
}

// static
LRESULT CALLBACK WinMessageLoopAttachment::WindowProc(HWND hWnd,
                                                      UINT uMsg,
                                                      WPARAM wParam,
                                                      LPARAM lParam) {
  if (uMsg == WM_LIBBASE_EXECUTE_TASK) {
    DCHECK_NE(g_current_instance, nullptr);

    const MessagePump::ExecutorId executor_id = 0;
    if (auto pending_task =
            g_current_instance->message_pump_->GetNextPendingTask(executor_id,
                                                                  false)) {
      std::move(pending_task.task).Run();
    }
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

}  // namespace win
}  // namespace base
