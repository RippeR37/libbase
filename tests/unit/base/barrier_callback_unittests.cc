#include "base/barrier_callback.h"

#include <array>
#include <vector>

#include "gtest/gtest.h"

#include "base/threading/thread.h"

namespace {

TEST(BarrierCallbackTest, ZeroCount) {
  bool verification_done = false;
  auto verify_callback = base::BindOnce(
      [](bool* done_flag, const std::vector<int>& results) {
        *done_flag = true;
        EXPECT_TRUE(results.empty());
      },
      &verification_done);

  auto barrier_callback =
      base::BarrierCallback<int>(0, std::move(verify_callback));
  EXPECT_FALSE(barrier_callback);
  EXPECT_TRUE(verification_done);
}

TEST(BarrierCallbackTest, SingleRun) {
  bool verification_done = false;
  auto verify_callback = base::BindOnce(
      [](bool* done_flag, const std::vector<int>& results) {
        *done_flag = true;
        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results[0], 3);
      },
      &verification_done);

  auto barrier_callback =
      base::BarrierCallback<int>(1, std::move(verify_callback));
  EXPECT_TRUE(barrier_callback);
  EXPECT_FALSE(verification_done);

  barrier_callback.Run(3);
  EXPECT_TRUE(barrier_callback);
  EXPECT_TRUE(verification_done);
}

TEST(BarrierCallbackTest, MultipleRunsMoveOnlyObjectsSingleThread) {
  constexpr size_t kRunsCount = 10;

  bool verification_done = false;
  auto verify_callback = base::BindOnce(
      [](bool* done_flag, size_t expected_element_count,
         std::vector<std::unique_ptr<int>> results) {
        *done_flag = true;
        ASSERT_EQ(results.size(), expected_element_count);
        for (size_t idx = 0; idx < expected_element_count; ++idx) {
          EXPECT_EQ(*results[idx], idx);
        }
      },
      &verification_done, kRunsCount);

  auto barrier_callback = base::BarrierCallback<std::unique_ptr<int>>(
      kRunsCount, std::move(verify_callback));
  EXPECT_TRUE(barrier_callback);
  EXPECT_FALSE(verification_done);

  for (size_t idx = 0; idx < kRunsCount; ++idx) {
    barrier_callback.Run(std::make_unique<int>(static_cast<int>(idx)));
  }

  EXPECT_TRUE(verification_done);
}

TEST(BarrierCallbackTest, MultipleRunsMultiThread) {
  constexpr size_t kRunsCount = 10;

  bool verification_done = false;
  auto verify_callback = base::BindOnce(
      [](bool* done_flag, size_t expected_element_count,
         std::vector<std::thread::id> results) {
        *done_flag = true;
        ASSERT_EQ(results.size(), expected_element_count);
        EXPECT_EQ(results.back(), std::this_thread::get_id());
      },
      &verification_done, kRunsCount);

  std::array<base::Thread, kRunsCount> threads;
  auto barrier_callback = base::BarrierCallback<std::thread::id>(
      kRunsCount, std::move(verify_callback));

  for (auto& thread : threads) {
    thread.Start();
  }

  for (auto& thread : threads) {
    thread.TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(
            [](base::RepeatingCallback<void(std::thread::id)> callback) {
              callback.Run(std::this_thread::get_id());
            },
            barrier_callback));
  }

  // This ensures that all posted callbacks have been executed
  for (auto& thread : threads) {
    thread.Stop();
  }

  EXPECT_TRUE(verification_done);
}

}  // namespace
