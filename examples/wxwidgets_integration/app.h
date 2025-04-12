#pragma once

#include <memory>

#include "base/message_loop/wx/wx_message_loop_attachment.h"
#include "base/threading/thread.h"

#include "wx/app.h"

class IntegrationExample : public wxApp {
 public:
  bool OnInit() override;
  int OnExit() override;

 private:
  std::unique_ptr<base::wx::WxMessageLoopAttachment> message_loop_attachment_;
  base::Thread worker_thread_;
};
