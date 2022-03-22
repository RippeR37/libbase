#pragma once

#include <cstdint>

#include "base/time/time_delta.h"

namespace base {

class TimeTicks {
 public:
  static TimeTicks Now();

  TimeTicks() = default;

  bool operator==(TimeTicks other) const;
  bool operator!=(TimeTicks other) const;
  bool operator<(TimeTicks other) const;
  bool operator>(TimeTicks other) const;
  bool operator<=(TimeTicks other) const;
  bool operator>=(TimeTicks other) const;

  TimeTicks operator+(TimeDelta delta) const;
  TimeTicks operator-(TimeDelta delta) const;
  TimeTicks& operator+=(TimeDelta delta);
  TimeTicks& operator-=(TimeDelta delta);

  TimeDelta operator-(TimeTicks other) const;

 private:
  TimeTicks(int64_t us_time) : us_time_(us_time) {}

  int64_t us_time_;
};

}  // namespace base
