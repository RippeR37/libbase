#pragma once

namespace base {

class MessageLoop {
 public:
  virtual ~MessageLoop() = default;

  virtual bool RunOnce() = 0;
  virtual void RunUntilIdle() = 0;

  virtual void Run() = 0;
  virtual void Stop() = 0;
};

}  // namespace base
