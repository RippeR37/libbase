#include <atomic>
#include <future>
#include <thread>

#include "base/callback_helpers.h"
#include "base/message_loop/message_pump_impl.h"

#include "gtest/gtest.h"

namespace {

const base::MessagePump::ExecutorId kExecutorId = 0;
const base::MessagePump::ExecutorId kOtherExecutorId = 1;
const base::MessagePump::ExecutorId kHighestExecutorId = kOtherExecutorId;
const size_t kExecutorCount = kHighestExecutorId + 1;

base::MessagePump::PendingTask CreateExecutorTask(
    base::OnceClosure task,
    std::optional<base::MessagePump::ExecutorId> executor_id) {
  return {std::move(task), {}, std::move(executor_id)};
}

base::MessagePump::PendingTask CreateSequenceTask(
    base::OnceClosure task,
    std::optional<base::SequenceId> sequence_id) {
  return {std::move(task), std::move(sequence_id), {}};
}

base::MessagePump::PendingTask CreateTask(base::OnceClosure task) {
  return CreateExecutorTask(std::move(task), {});
}

base::MessagePump::PendingTask CreateSetterTask(bool& flag) {
  EXPECT_FALSE(flag);
  return CreateTask(
      base::BindOnce([](bool* ext_flag) { *ext_flag = true; }, &flag));
}

base::MessagePump::PendingTask CreateSetterExecutorTask(
    std::optional<base::MessagePump::ExecutorId> executor_id,
    bool& flag) {
  EXPECT_FALSE(flag);
  return CreateExecutorTask(
      base::BindOnce([](bool* ext_flag) { *ext_flag = true; }, &flag),
      std::move(executor_id));
}

base::MessagePump::PendingTask CreateSetterSequenceTask(
    std::optional<base::SequenceId> sequence_id,
    bool& flag) {
  EXPECT_FALSE(flag);
  return CreateSequenceTask(
      base::BindOnce([](bool* ext_flag) { *ext_flag = true; }, &flag),
      std::move(sequence_id));
}

class MessagePumpImplTest : public ::testing::Test {
 public:
  MessagePumpImplTest() : pump(kExecutorCount) {}

  base::MessagePumpImpl pump;
};

TEST_F(MessagePumpImplTest, NoTasksAfterStopEmptyQueue) {
  pump.Stop();
  EXPECT_FALSE(pump.GetNextPendingTask(kExecutorId));
}

TEST_F(MessagePumpImplTest, RemainingTasksAfterStopNonEmptyQueue) {
  EXPECT_TRUE(pump.QueuePendingTask(CreateTask(base::DoNothing{})));
  pump.Stop();
  EXPECT_TRUE(pump.GetNextPendingTask(kExecutorId));
  EXPECT_FALSE(pump.GetNextPendingTask(kExecutorId));
}

TEST_F(MessagePumpImplTest, NoTasksAfterStopAndTaskQueued) {
  pump.Stop();
  EXPECT_FALSE(pump.QueuePendingTask(CreateTask(base::DoNothing{})));
  EXPECT_FALSE(pump.GetNextPendingTask(kExecutorId));
}

TEST_F(MessagePumpImplTest, DequeueInCorrectOrder) {
  bool task1_flag = false;
  bool task2_flag = false;
  bool task3_flag = false;

  EXPECT_TRUE(pump.QueuePendingTask(CreateSetterTask(task1_flag)));
  EXPECT_TRUE(pump.QueuePendingTask(CreateSetterTask(task2_flag)));

  auto task1 = pump.GetNextPendingTask(kExecutorId);
  EXPECT_FALSE(task1_flag);
  EXPECT_FALSE(task2_flag);
  EXPECT_FALSE(task3_flag);

  std::move(task1.task).Run();
  EXPECT_TRUE(task1_flag);
  EXPECT_FALSE(task2_flag);
  EXPECT_FALSE(task3_flag);

  EXPECT_TRUE(pump.QueuePendingTask(CreateSetterTask(task3_flag)));

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

  EXPECT_TRUE(pump.QueuePendingTask(
      CreateExecutorTask(base::DoNothing{}, kOtherExecutorId)));
  EXPECT_TRUE(pump.QueuePendingTask(
      CreateSetterExecutorTask(kExecutorId, allowed_executor_task_executed)));
  EXPECT_TRUE(pump.QueuePendingTask(
      CreateSetterExecutorTask({}, any_executor_task_executed)));

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

TEST_F(MessagePumpImplTest, DequeueSkipsTasksFromActiveSequences) {
  const auto sequence_1 =
      base::detail::SequenceIdGenerator::GetNextSequenceId();
  const auto sequence_2 =
      base::detail::SequenceIdGenerator::GetNextSequenceId();

  bool task1_sequence1 = false;
  bool task2_sequence1 = false;
  bool task3_sequence2 = false;

  EXPECT_TRUE(pump.QueuePendingTask(
      CreateSetterSequenceTask(sequence_1, task1_sequence1)));
  EXPECT_TRUE(pump.QueuePendingTask(
      CreateSetterSequenceTask(sequence_1, task2_sequence1)));
  EXPECT_TRUE(pump.QueuePendingTask(
      CreateSetterSequenceTask(sequence_2, task3_sequence2)));

  auto task1 = pump.GetNextPendingTask(kExecutorId);
  EXPECT_FALSE(task1.allowed_executor_id);
  ASSERT_TRUE(task1.task);
  std::move(task1.task).Run();
  EXPECT_TRUE(task1_sequence1);
  EXPECT_FALSE(task2_sequence1);
  EXPECT_FALSE(task3_sequence2);

  // This simulates another executor thread asking for available work.
  auto task2 = pump.GetNextPendingTask(kOtherExecutorId);
  EXPECT_FALSE(task2.allowed_executor_id);
  ASSERT_TRUE(task2.task);
  std::move(task2.task).Run();
  EXPECT_TRUE(task1_sequence1);
  EXPECT_FALSE(task2_sequence1);
  EXPECT_TRUE(task3_sequence2);
}

TEST_F(MessagePumpImplTest, DequeueOnEmptyPumpWaitsForStop) {
  using namespace std::chrono_literals;

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
  using namespace std::chrono_literals;

  bool task_executed = false;
  std::atomic_bool dequeue_finished = false;

  const auto async_result = std::async(std::launch::async, [&]() {
    std::this_thread::sleep_for(20ms);
    EXPECT_FALSE(dequeue_finished);
    EXPECT_TRUE(pump.QueuePendingTask(CreateSetterTask(task_executed)));
  });
  auto result = pump.GetNextPendingTask(kExecutorId);
  dequeue_finished = true;
  EXPECT_TRUE(result);
  ASSERT_TRUE(result.task);

  EXPECT_FALSE(task_executed);
  std::move(result.task).Run();
  EXPECT_TRUE(task_executed);
}

TEST_F(MessagePumpImplTest, DequeueOnBlockedSequencedWaitsForStop) {
  using namespace std::chrono_literals;

  const auto sequence_id =
      base::detail::SequenceIdGenerator::GetNextSequenceId();
  EXPECT_TRUE(pump.QueuePendingTask(
      CreateSequenceTask(base::DoNothing{}, sequence_id)));
  EXPECT_TRUE(pump.QueuePendingTask(
      CreateSequenceTask(base::DoNothing{}, sequence_id)));

  auto task1 = pump.GetNextPendingTask(kExecutorId);
  std::atomic_bool dequeue_finished = false;
  const auto async_result = std::async(std::launch::async, [&]() {
    std::this_thread::sleep_for(20ms);
    EXPECT_FALSE(dequeue_finished);
    pump.Stop();
  });
  const auto result = pump.GetNextPendingTask(kOtherExecutorId);
  dequeue_finished = true;
  EXPECT_FALSE(result);
}

}  // namespace
