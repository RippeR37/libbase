#pragma once

#include <memory>
#include <optional>
#include <thread>

#include "base/callback_forward.h"
#include "base/message_loop/message_loop.h"
#include "base/sequence_id.h"
#include "base/single_thread_task_runner.h"
#include "base/source_location.h"

namespace base {

class Thread {
 public:
  Thread();
  ~Thread();

  void Start();
  void Stop();
  void Stop(SourceLocation location, OnceClosure last_task);

  std::thread::id Id() const;
  std::shared_ptr<SingleThreadTaskRunner> TaskRunner();

  void FlushForTesting();

 private:
  std::unique_ptr<MessageLoop> message_loop_;
  std::unique_ptr<std::thread> thread_;
  std::optional<base::SequenceId> sequence_id_;
  std::shared_ptr<SingleThreadTaskRunner> task_runner_;
};

}  // namespace base
