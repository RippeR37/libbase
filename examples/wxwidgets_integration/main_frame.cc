#include "main_frame.h"

#include "wx/msgdlg.h"
#include "wx/panel.h"

MainFrame::MainFrame(std::shared_ptr<base::SequencedTaskRunner> task_runner)
    : wxFrame(nullptr,
              wxID_ANY,
              "wxWidgets integration example",
              wxDefaultPosition,
              wxSize(400, 200)),
      task_runner_(std::move(task_runner)),
      weak_factory_(this) {
  weak_this_ = weak_factory_.GetWeakPtr();

  auto* panel = new wxPanel(this, wxID_ANY);
  m_button = new wxButton(panel, wxID_ANY, "Click Me", wxPoint(150, 70),
                          wxDefaultSize);

  m_button->Bind(wxEVT_BUTTON, &MainFrame::OnButtonClicked, this);
}

void MainFrame::OnButtonClicked(wxCommandEvent&) {
  DCHECK(!task_runner_->RunsTasksInCurrentSequence());

  task_runner_->PostTaskAndReply(
      FROM_HERE, base::BindOnce([]() {
        // some work on worker thread
      }),
      base::BindOnce(&MainFrame::ShowMessageBox, weak_this_));
}

void MainFrame::ShowMessageBox() {
  DCHECK(!task_runner_->RunsTasksInCurrentSequence());

  wxMessageBox("Hello world!", "Message", wxOK | wxICON_INFORMATION);
}
