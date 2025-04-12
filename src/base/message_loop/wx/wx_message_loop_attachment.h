#pragma once

#include <memory>

#include "base/message_loop/message_pump.h"
#include "base/sequence_id.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/sequenced_task_runner_handle.h"

class wxEvtHandler;

namespace base {
namespace wx {

namespace detail {
class WxExecuteTaskEvent;
}  // namespace detail

class WxMessageLoopAttachment {
 public:
  WxMessageLoopAttachment(wxEvtHandler* event_handler);
  ~WxMessageLoopAttachment();

  std::shared_ptr<SingleThreadTaskRunner> TaskRunner() const;

 private:
  static void HandleExecuteTaskEvent(detail::WxExecuteTaskEvent& event);

  void BindEventHandling();
  void UnbindEventHandling();

  wxEvtHandler* event_handler_;
  SequenceId sequence_id_;
  std::shared_ptr<MessagePump> message_pump_;
  std::shared_ptr<SingleThreadTaskRunner> task_runner_;
  base::detail::ScopedSequenceIdSetter scoped_sequence_id_;
  SequencedTaskRunnerHandle scoped_task_runner_handle_;
};

}  // namespace wx
}  // namespace base
