#include <atomic>
#include <future>
#include <thread>

#include "base/bind_helpers.h"
#include "base/message_loop/message_pump_impl.h"

#include "gtest/gtest.h"

namespace {

using namespace ::testing;

const base::MessagePump::ExecutorId kExecutorId = 0;
const base::MessagePump::ExecutorId kOtherExecutorId = 1;

class MessagePumpImplTest : public Test {
 public:
  void SetUp() override {
    //
  }

  base::MessagePump::PendingTask CreateExecutorTask(
      base::OnceClosure task,
      std::optional<base::MessagePump::ExecutorId> executor_id) {
    return {std::move(task), {}, std::move(executor_id)};
  }

  base::MessagePump::PendingTask CreateTask(base::OnceClosure task) {
    return CreateExecutorTask(std::move(task), {});
  }

  base::MessagePump::PendingTask CreateEmptyTask() { return CreateTask({}); }

  base::MessagePump::PendingTask CreateSetterTask(bool& flag) {
    EXPECT_FALSE(flag);
    return CreateTask(
        base::BindOnce([](bool& ext_flag) { ext_flag = true; }, flag));
  }

  base::MessagePump::PendingTask CreateSetterExecutorTask(
      std::optional<base::MessagePump::ExecutorId> executor_id,
      bool& flag) {
    EXPECT_FALSE(flag);
    return CreateExecutorTask(
        base::BindOnce([](bool& ext_flag) { ext_flag = true; }, flag),
        std::move(executor_id));
  }

  base::MessagePumpImpl pump;
};

TEST_F(MessagePumpImplTest, NoTasksAfterStopEmptyQueue) {
  pump.Stop();
  EXPECT_FALSE(pump.GetNextPendingTask(kExecutorId));
}

TEST_F(MessagePumpImplTest, NoTasksAfterStopNonEmptyQueue) {
  pump.QueuePendingTask(CreateTask(base::DoNothing{}));
  pump.Stop();
  EXPECT_FALSE(pump.GetNextPendingTask(kExecutorId));
}

TEST_F(MessagePumpImplTest, NoTasksAfterStopAndTaskQueued) {
  pump.Stop();
  pump.QueuePendingTask(CreateTask(base::DoNothing{}));
  EXPECT_FALSE(pump.GetNextPendingTask(kExecutorId));
}

TEST_F(MessagePumpImplTest, DequeueInCorrectOrder) {
  bool task1_flag = false;
  bool task2_flag = false;
  bool task3_flag = false;

  pump.QueuePendingTask(CreateSetterTask(task1_flag));
  pump.QueuePendingTask(CreateSetterTask(task2_flag));

  auto task1 = pump.GetNextPendingTask(kExecutorId);
  EXPECT_FALSE(task1_flag);
  EXPECT_FALSE(task2_flag);
  EXPECT_FALSE(task3_flag);

  std::move(task1.task).Run();
  EXPECT_TRUE(task1_flag);
  EXPECT_FALSE(task2_flag);
  EXPECT_FALSE(task3_flag);

  pump.QueuePendingTask(CreateSetterTask(task3_flag));

  auto task2 = pump.GetNextPendingTask(kExecutorId);
  auto task3 = pump.GetNextPendingTask(kExecutorId);
  EXPECT_FALSE(task2_flag);
  EXPECT_FALSE(task3_flag);

  std::move(task2.task).Run();
  EXPECT_TRUE(task2_flag);
  EXPECT_FALSE(task3_flag);

  std::move(task3.task).Run();
  EXPECT_TRUE(task3_flag);
}

TEST_F(MessagePumpImplTest, DequeueOnlyForAllowedExecutor) {
  bool allowed_executor_task_executed = false;
  bool any_executor_task_executed = false;

  pump.QueuePendingTask(
      CreateExecutorTask(base::DoNothing{}, kOtherExecutorId));
  pump.QueuePendingTask(
      CreateSetterExecutorTask(kExecutorId, allowed_executor_task_executed));
  pump.QueuePendingTask(
      CreateSetterExecutorTask({}, any_executor_task_executed));

  auto task1 = pump.GetNextPendingTask(kExecutorId);
  ASSERT_TRUE(task1.allowed_executor_id);
  EXPECT_EQ(*task1.allowed_executor_id, kExecutorId);
  ASSERT_TRUE(task1.task);
  std::move(task1.task).Run();
  EXPECT_TRUE(allowed_executor_task_executed);
  EXPECT_FALSE(any_executor_task_executed);

  auto task2 = pump.GetNextPendingTask(kExecutorId);
  EXPECT_FALSE(task2.allowed_executor_id);
  ASSERT_TRUE(task2.task);
  std::move(task2.task).Run();
  EXPECT_TRUE(allowed_executor_task_executed);
  EXPECT_TRUE(any_executor_task_executed);
}

TEST_F(MessagePumpImplTest, DequeueOnEmptyPumpWaitsForStop) {
  using std::chrono_literals::operator""ms;

  std::atomic_bool dequeue_finished = false;
  const auto async_result = std::async(std::launch::async, [&]() {
    std::this_thread::sleep_for(20ms);
    EXPECT_FALSE(dequeue_finished);
    pump.Stop();
  });
  const auto result = pump.GetNextPendingTask(kExecutorId);
  dequeue_finished = true;
  EXPECT_FALSE(result);
}

TEST_F(MessagePumpImplTest, DequeueOnEmptyPumpWaitsForEnqueue) {
  using std::chrono_literals::operator""ms;

  bool task_executed = false;
  std::atomic_bool dequeue_finished = false;

  const auto async_result = std::async(std::launch::async, [&]() {
    std::this_thread::sleep_for(20ms);
    EXPECT_FALSE(dequeue_finished);
    pump.QueuePendingTask(CreateSetterTask(task_executed));
  });
  auto result = pump.GetNextPendingTask(kExecutorId);
  dequeue_finished = true;
  EXPECT_TRUE(result);
  ASSERT_TRUE(result.task);

  EXPECT_FALSE(task_executed);
  std::move(result.task).Run();
  EXPECT_TRUE(task_executed);
}

}  // namespace
