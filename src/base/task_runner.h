#pragma once

#include "base/callback.h"
#include "base/source_location.h"

namespace base {

class TaskRunner {
 public:
  virtual ~TaskRunner() = default;

  virtual bool PostTask(SourceLocation location, OnceCallback<void()> task) = 0;
};

}  // namespace base
