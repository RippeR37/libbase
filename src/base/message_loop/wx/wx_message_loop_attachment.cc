#include "base/message_loop/wx/wx_message_loop_attachment.h"

#include <atomic>

#include "base/threading/delayed_task_manager_shared_instance.h"
#include "base/threading/task_runner_impl.h"

#include "wx/event.h"

namespace base {
namespace wx {

namespace detail {

//
// WxExecuteTaskEvent
//

class WxExecuteTaskEvent;
wxDECLARE_EVENT(wxEVT_LIBBASE_EXECUTE_TASK, WxExecuteTaskEvent);

class WxExecuteTaskEvent : public wxCommandEvent {
 public:
  WxExecuteTaskEvent(std::shared_ptr<MessagePump::PendingTask> pending_task,
                     wxEventType eventType = wxEVT_LIBBASE_EXECUTE_TASK)
      : wxCommandEvent(eventType), pending_task_(std::move(pending_task)) {}

  WxExecuteTaskEvent(const WxExecuteTaskEvent& other)
      : WxExecuteTaskEvent(other.pending_task_, other.GetEventType()) {}

  wxEvent* Clone() const override { return new WxExecuteTaskEvent(*this); }

  std::shared_ptr<MessagePump::PendingTask> pending_task_;
};

using MyWxExecuteTaskEventFunction =
    void (wxEvtHandler::*)(WxExecuteTaskEvent&);
#define MyWxExecuteTaskEventHandler(func) \
  wxEVENT_HANDLER_CAST(MyWxExecuteTaskEventFunction, func)

wxDEFINE_EVENT(wxEVT_LIBBASE_EXECUTE_TASK, WxExecuteTaskEvent);

//
// WxMessagePumpImpl
//

class WxMessagePumpImpl : public MessagePump {
 public:
  WxMessagePumpImpl(wxEvtHandler* event_handler)
      : event_handler_(event_handler), is_stopped_(false) {}

  PendingTask GetNextPendingTask(ExecutorId, bool) override {
    DCHECK(false) << "This method should not be called";
    return {};
  }

  bool QueuePendingTask(PendingTask pending_task) override {
    if (!is_stopped_) {
      wxPostEvent(event_handler_,
                  WxExecuteTaskEvent(std::make_shared<MessagePump::PendingTask>(
                      std::move(pending_task))));
      return true;
    }
    return false;
  }

  void Stop(PendingTask last_task) override {
    QueuePendingTask(std::move(last_task));
    is_stopped_ = true;
  }

 private:
  wxEvtHandler* event_handler_;
  std::atomic_bool is_stopped_;
};

}  // namespace detail

//
// WxMessageLoopAttachment
//

WxMessageLoopAttachment::WxMessageLoopAttachment(wxEvtHandler* event_handler)
    : event_handler_(event_handler),
      sequence_id_(base::detail::SequenceIdGenerator::GetNextSequenceId()),
      message_pump_(
          std::make_shared<detail::WxMessagePumpImpl>(event_handler_)),
      task_runner_(SingleThreadTaskRunnerImpl::Create(
          message_pump_,
          sequence_id_,
          0,
          DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance())),
      scoped_sequence_id_(sequence_id_),
      scoped_task_runner_handle_(task_runner_) {
  DCHECK(event_handler_);

  BindEventHandling();
}

WxMessageLoopAttachment::~WxMessageLoopAttachment() {
  UnbindEventHandling();
  message_pump_->Stop({});
}

std::shared_ptr<SingleThreadTaskRunner> WxMessageLoopAttachment::TaskRunner()
    const {
  return task_runner_;
}

// static
void WxMessageLoopAttachment::HandleExecuteTaskEvent(
    detail::WxExecuteTaskEvent& event) {
  // We don't have to set scoped sequence id/task runner handle here as this is
  // handled by the lifetime of owned object.
  if (event.pending_task_) {
    std::move(event.pending_task_->task).Run();
  }
}

void WxMessageLoopAttachment::BindEventHandling() {
  event_handler_->Bind(detail::wxEVT_LIBBASE_EXECUTE_TASK,
                       &WxMessageLoopAttachment::HandleExecuteTaskEvent);
}

void WxMessageLoopAttachment::UnbindEventHandling() {
  event_handler_->Unbind(detail::wxEVT_LIBBASE_EXECUTE_TASK,
                         &WxMessageLoopAttachment::HandleExecuteTaskEvent);
}

}  // namespace wx
}  // namespace base
