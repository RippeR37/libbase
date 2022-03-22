#include "base/time/time_ticks.h"

#include <chrono>

namespace base {

// static
TimeTicks TimeTicks::Now() {
  auto now = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      now.time_since_epoch());
  return TimeTicks{duration.count()};
}

bool TimeTicks::operator==(TimeTicks other) const {
  return us_time_ == other.us_time_;
}

bool TimeTicks::operator!=(TimeTicks other) const {
  return us_time_ != other.us_time_;
}

bool TimeTicks::operator<(TimeTicks other) const {
  return us_time_ < other.us_time_;
}

bool TimeTicks::operator>(TimeTicks other) const {
  return us_time_ > other.us_time_;
}

bool TimeTicks::operator<=(TimeTicks other) const {
  return us_time_ <= other.us_time_;
}

bool TimeTicks::operator>=(TimeTicks other) const {
  return us_time_ >= other.us_time_;
}

TimeTicks TimeTicks::operator+(TimeDelta delta) const {
  return TimeTicks{us_time_ + delta.InMicroseconds()};
}

TimeTicks TimeTicks::operator-(TimeDelta delta) const {
  return TimeTicks{us_time_ - delta.InMicroseconds()};
}

TimeTicks& TimeTicks::operator+=(TimeDelta delta) {
  us_time_ += delta.InMicroseconds();
  return *this;
}

TimeTicks& TimeTicks::operator-=(TimeDelta delta) {
  us_time_ -= delta.InMicroseconds();
  return *this;
}

TimeDelta TimeTicks::operator-(TimeTicks other) const {
  return TimeDelta{us_time_ - other.us_time_};
}

}  // namespace base
