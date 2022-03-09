#include "base/barrier_closure.h"

#include <array>
#include <vector>

#include "gtest/gtest.h"

#include "base/threading/thread.h"

namespace {

TEST(BarrierCallbackTest, ZeroCount) {
  bool verification_done = false;
  auto verify_closure = base::BindOnce(
      [](bool* done_flag) { *done_flag = true; }, &verification_done);

  auto barrier_closure = base::BarrierClosure(0, std::move(verify_closure));
  EXPECT_FALSE(barrier_closure);
  EXPECT_TRUE(verification_done);
}

TEST(BarrierCallbackTest, SingleRun) {
  bool verification_done = false;
  auto verify_closure = base::BindOnce(
      [](bool* done_flag) { *done_flag = true; }, &verification_done);

  auto barrier_closure = base::BarrierClosure(1, std::move(verify_closure));
  EXPECT_TRUE(barrier_closure);
  EXPECT_FALSE(verification_done);

  barrier_closure.Run();
  EXPECT_TRUE(barrier_closure);
  EXPECT_TRUE(verification_done);
}

TEST(BarrierCallbackTest, MultipleRuns) {
  constexpr size_t kRunsCount = 10;

  bool verification_done = false;
  auto verify_closure = base::BindOnce(
      [](bool* done_flag) { *done_flag = true; }, &verification_done);

  auto barrier_closure =
      base::BarrierClosure(kRunsCount, std::move(verify_closure));
  EXPECT_TRUE(barrier_closure);
  EXPECT_FALSE(verification_done);

  for (size_t idx = 0; idx < kRunsCount; ++idx) {
    barrier_closure.Run();
  }
  EXPECT_TRUE(barrier_closure);
  EXPECT_FALSE(verification_done);

  barrier_closure.Run();
  EXPECT_TRUE(verification_done);
}

TEST(BarrierCallbackTest, MultipleRunsMultiThread) {
  constexpr size_t kRunsCount = 10;

  bool verification_done = false;
  auto verify_closure = base::BindOnce(
      [](bool* done_flag) { *done_flag = true; }, &verification_done);

  std::array<base::Thread, kRunsCount> threads;
  auto barrier_closure =
      base::BarrierClosure(kRunsCount, std::move(verify_closure));

  for (auto& thread : threads) {
    thread.Start();
  }

  for (auto& thread : threads) {
    thread.TaskRunner()->PostTask(FROM_HERE, barrier_closure);
  }

  // This ensures that all posted callbacks have been executed
  for (auto& thread : threads) {
    thread.Join();
  }

  EXPECT_TRUE(verification_done);
}

}  // namespace
