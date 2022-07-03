#include "base/timer/elapsed_timer.h"

namespace base {

ElapsedTimer::ElapsedTimer() : begin_(base::TimeTicks::Now()) {}

TimeDelta ElapsedTimer::Elapsed() const {
  return base::TimeTicks::Now() - Begin();
}

TimeTicks ElapsedTimer::Begin() const {
  return begin_;
}

}  // namespace base
