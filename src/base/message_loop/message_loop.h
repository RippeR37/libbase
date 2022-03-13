#pragma once

#include "base/callback_forward.h"

namespace base {

class MessageLoop {
 public:
  virtual ~MessageLoop() = default;

  virtual bool RunOnce() = 0;
  virtual void RunUntilIdle() = 0;

  virtual void Run() = 0;
  virtual void Stop(OnceClosure last_task) = 0;
};

}  // namespace base
