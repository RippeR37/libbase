#pragma once

#include <memory>
#include <thread>

#include "base/message_loop/message_loop.h"
#include "base/single_thread_task_runner.h"

namespace base {

class Thread {
 public:
  Thread();

  void Start();
  void Join();

  std::thread::id Id() const;
  std::shared_ptr<SingleThreadTaskRunner> TaskRunner();

 private:
  std::unique_ptr<MessageLoop> message_loop_;
  std::unique_ptr<std::thread> thread_;
  std::shared_ptr<SingleThreadTaskRunner> task_runner_;
};

}  // namespace base
