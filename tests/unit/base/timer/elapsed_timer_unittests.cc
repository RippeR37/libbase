#include "base/timer/elapsed_timer.h"

#include <chrono>
#include <memory>
#include <thread>

#include "gtest/gtest.h"

namespace {

TEST(ElapsedTimerTest, NonNegative) {
  const size_t kRepeats = 100;

  for (size_t repeat = 0; repeat < kRepeats; ++repeat) {
    base::ElapsedTimer timer;
    EXPECT_FALSE(timer.Elapsed().IsNegative());
  }
}

TEST(ElapsedTimerTest, AtLeastSleepValue) {
  base::ElapsedTimer timer;
  const auto elapsed_1 = timer.Elapsed();
  EXPECT_FALSE(elapsed_1.IsNegative());

  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  const auto elapsed_2 = timer.Elapsed();
  EXPECT_FALSE(elapsed_2.IsNegative());

  const auto diff = elapsed_2 - elapsed_1;
  EXPECT_GE(diff.InMillisecondsF(), 20.0);
}

}  // namespace
