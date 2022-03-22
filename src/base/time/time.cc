#include "base/time/time.h"

#if defined(LIBBASE_IS_LINUX)
#else  // defined(LIBBASE_IS_LINUX)
#include <chrono>
#endif  // defined(LIBBASE_IS_LINUX)

namespace base {

namespace {

const int64_t kMicrosecondsInSeconds = 1000 * 1000;
const int64_t kNanosecondsInMicroseconds = 1000;

#if defined(LIBBASE_IS_LINUX)
Time TimeSpecNow() {
  timespec now{};
  if (timespec_get(&now, TIME_UTC) == TIME_UTC) {
    return Time::FromTimeSpec(now);
  }
  return {};
}
#else   // defined(LIBBASE_IS_LINUX)
Time GenericNow() {
  return Time::FromTimeT(
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}
#endif  // defined(LIBBASE_IS_LINUX)

}  // namespace

// static
Time Time::Now() {
#if defined(LIBBASE_IS_LINUX)
  return TimeSpecNow();
#else
  return GenericNow();
#endif
}

// static
Time Time::FromTimeT(time_t tt) {
  return Time{tt * kMicrosecondsInSeconds};
}

time_t Time::ToTimeT() const {
  return static_cast<time_t>(us_time_ / kMicrosecondsInSeconds);
}

#if defined(LIBBASE_IS_LINUX)
// static
Time Time::FromTimeSpec(const timespec& ts) {
  return Time{ts.tv_sec * kMicrosecondsInSeconds +
              ts.tv_nsec / kNanosecondsInMicroseconds};
}

timespec Time::ToTimeSpec() const {
  timespec result{};
  result.tv_sec = us_time_ / kMicrosecondsInSeconds;
  result.tv_nsec = static_cast<long>((us_time_ % kMicrosecondsInSeconds) *
                                     kNanosecondsInMicroseconds);
  return result;
}
#endif  // defined(LIBBASE_IS_LINUX)

bool Time::operator==(Time other) const {
  return us_time_ == other.us_time_;
}

bool Time::operator!=(Time other) const {
  return us_time_ != other.us_time_;
}

bool Time::operator<(Time other) const {
  return us_time_ < other.us_time_;
}

bool Time::operator>(Time other) const {
  return us_time_ > other.us_time_;
}

bool Time::operator<=(Time other) const {
  return us_time_ <= other.us_time_;
}

bool Time::operator>=(Time other) const {
  return us_time_ >= other.us_time_;
}

Time Time::operator+(TimeDelta delta) const {
  return Time{us_time_ + delta.InMicroseconds()};
}

Time Time::operator-(TimeDelta delta) const {
  return Time{us_time_ - delta.InMicroseconds()};
}

Time& Time::operator+=(TimeDelta delta) {
  us_time_ += delta.InMicroseconds();
  return *this;
}

Time& Time::operator-=(TimeDelta delta) {
  us_time_ -= delta.InMicroseconds();
  return *this;
}

TimeDelta Time::operator-(Time other) const {
  return TimeDelta{us_time_ - other.us_time_};
}

}  // namespace base
