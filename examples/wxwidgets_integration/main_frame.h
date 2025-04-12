#pragma once

#include <memory>

#include "base/sequenced_task_runner.h"
#include "base/synchronization/waitable_event.h"

#include "wx/button.h"
#include "wx/frame.h"

class MainFrame : public wxFrame {
 public:
  MainFrame(std::shared_ptr<base::SequencedTaskRunner> task_runner);

 private:
  void OnButtonClicked(wxCommandEvent& event);
  void ShowMessageBox();

  std::shared_ptr<base::SequencedTaskRunner> task_runner_;
  wxButton* m_button;

  base::WeakPtr<MainFrame> weak_this_;
  base::WeakPtrFactory<MainFrame> weak_factory_{this};
};
