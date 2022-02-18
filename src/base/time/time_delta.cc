#include "base/time/time_delta.h"

namespace base {

#if defined(LIBBASE_IS_LINUX)
// static
TimeDelta TimeDelta::FromTimeSpec(const timespec& ts) {
  return TimeDelta{ts.tv_sec * kMicrosecondsInSeconds +
                   ts.tv_nsec / kNanosecondsInMicroseconds};
}
#endif  // defined(LIBBASE_IS_LINUX)

bool TimeDelta::IsZero() const {
  return us_delta_ == 0;
}

bool TimeDelta::IsPositive() const {
  return us_delta_ > 0;
}

bool TimeDelta::IsNegative() const {
  return us_delta_ < 0;
}

int TimeDelta::InDays() const {
  return static_cast<int>(us_delta_ / kMicrosecondsInDays);
}

int TimeDelta::InHours() const {
  return static_cast<int>(us_delta_ / kMicrosecondsInHours);
}

int TimeDelta::InMinutes() const {
  return static_cast<int>(us_delta_ / kMicrosecondsInMinutes);
}

double TimeDelta::InSecondsF() const {
  return static_cast<double>(us_delta_) / kMicrosecondsInSeconds;
}

int64_t TimeDelta::InSeconds() const {
  return us_delta_ / kMicrosecondsInSeconds;
}

double TimeDelta::InMillisecondsF() const {
  return static_cast<double>(us_delta_) / kMicrosecondsInMilliseconds;
}

int64_t TimeDelta::InMilliseconds() const {
  return us_delta_ / kMicrosecondsInMilliseconds;
}

int64_t TimeDelta::InMicroseconds() const {
  return us_delta_;
}

double TimeDelta::InMicrosecondsF() const {
  return static_cast<double>(us_delta_);
}

int64_t TimeDelta::InNanoseconds() const {
  return us_delta_ * kNanosecondsInMicroseconds;
}

}  // namespace base
