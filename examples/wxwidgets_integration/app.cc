#include "app.h"
#include "main_frame.h"

bool IntegrationExample::OnInit() {
  message_loop_attachment_ =
      std::make_unique<base::wx::WxMessageLoopAttachment>(this);
  worker_thread_.Start();

  auto* frame = new MainFrame(worker_thread_.TaskRunner());
  frame->Show(true);
  return true;
}

int IntegrationExample::OnExit() {
  worker_thread_.Stop();
  message_loop_attachment_.reset();
  return 0;
}
