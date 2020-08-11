#include "base/synchronization/waitable_event.h"

#include <chrono>
#include <future>
#include <thread>
#include <tuple>

#include "gtest/gtest.h"

namespace {

using namespace ::testing;

using ResetPolicy = base::WaitableEvent::ResetPolicy;
using InitialState = base::WaitableEvent::InitialState;

class WaitableEventTest
    : public Test,
      public WithParamInterface<std::tuple<ResetPolicy, InitialState>> {
 public:
  WaitableEventTest() : waitable_event_(GetResetPolicy(), GetInitialState()) {}

  ResetPolicy GetResetPolicy() { return std::get<ResetPolicy>(GetParam()); }
  InitialState GetInitialState() { return std::get<InitialState>(GetParam()); }

 protected:
  base::WaitableEvent waitable_event_;
};

TEST_P(WaitableEventTest, CorrectState) {
  const auto expected_initial_signaled =
      (GetInitialState() == InitialState::kSignaled);
  EXPECT_EQ(waitable_event_.IsSignaled(), expected_initial_signaled);

  const auto remained_signaled =
      (expected_initial_signaled && (GetResetPolicy() == ResetPolicy::kManual));
  EXPECT_EQ(waitable_event_.IsSignaled(), remained_signaled);

  waitable_event_.Reset();
  EXPECT_FALSE(waitable_event_.IsSignaled());

  waitable_event_.Signal();
  EXPECT_TRUE(waitable_event_.IsSignaled());

  const auto again_remained_signaled =
      (GetResetPolicy() == ResetPolicy::kManual);
  EXPECT_EQ(waitable_event_.IsSignaled(), again_remained_signaled);
}

TEST_P(WaitableEventTest, WaitOnSignaledFinishes) {
  if (GetInitialState() == InitialState::kNotSignaled) {
    waitable_event_.Signal();
  }

  waitable_event_.Wait();  // expected to finish right away

  if (GetResetPolicy() == ResetPolicy::kAutomatic) {
    EXPECT_FALSE(waitable_event_.IsSignaled());
    waitable_event_.Signal();
  }

  waitable_event_.Wait();  // expected to finish right away
}

TEST_P(WaitableEventTest, WaitBlocksWhenNotSignaled) {
  if (GetInitialState() == InitialState::kSignaled) {
    switch (GetResetPolicy()) {
      case base::WaitableEvent::ResetPolicy::kManual:
        waitable_event_.Reset();
        break;
      case base::WaitableEvent::ResetPolicy::kAutomatic:
        waitable_event_.Wait();
        break;
    }
  }

  // Expected: not signaled at this point

  std::atomic_bool wait_finished = false;
  std::atomic_bool signal_finished = false;
  auto async_result = std::async(std::launch::async, [&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(wait_finished);
    signal_finished = true;
    waitable_event_.Signal();
  });
  waitable_event_.Wait();
  wait_finished = true;
  EXPECT_TRUE(signal_finished);

  const auto expected_still_signaled =
      (GetResetPolicy() == ResetPolicy::kManual);
  EXPECT_EQ(waitable_event_.IsSignaled(), expected_still_signaled);
}

INSTANTIATE_TEST_SUITE_P(
    WaitableEventParameterizedTests,
    WaitableEventTest,
    Combine(Values(ResetPolicy::kManual, ResetPolicy::kAutomatic),
            Values(InitialState::kSignaled, InitialState::kNotSignaled)));

}  // namespace
