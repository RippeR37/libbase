#pragma once

#include "base/time/time_ticks.h"

namespace base {

class ElapsedTimer {
 public:
  ElapsedTimer();

  ElapsedTimer(ElapsedTimer&&) = default;
  ElapsedTimer& operator=(ElapsedTimer&&) = default;

  ElapsedTimer(const ElapsedTimer&) = delete;
  ElapsedTimer& operator=(const ElapsedTimer&) = delete;

  TimeDelta Elapsed() const;
  TimeTicks Begin() const;

 private:
  TimeTicks begin_;
};

}  // namespace base
