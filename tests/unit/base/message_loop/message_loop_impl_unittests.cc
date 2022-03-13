#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <thread>

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop_impl.h"
#include "base/message_loop/mock_message_pump.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/threading/sequenced_task_runner_handle.h"

#include "gtest/gtest.h"

namespace {

using ::testing::_;
using ::testing::ByMove;
using ::testing::Return;
using ::testing::Test;

class MessageLoopImplTest : public Test {
 public:
  void SetUp() override {
    const base::MessagePump::ExecutorId executor_id = 0;
    mock_message_pump_ = std::make_shared<MockMessagePump>();
    message_loop_impl_ = std::make_unique<base::MessageLoopImpl>(
        executor_id, mock_message_pump_);
  }

  void TearDown() override {
    message_loop_impl_.reset();
    mock_message_pump_.reset();
  }

  void SetupDefaultGetNextPendingTask() {
    EXPECT_CALL(*mock_message_pump_, GetNextPendingTask).WillRepeatedly([&]() {
      return CreateEmptyPendingTask();
    });
  }

  void ExpectTaskExecution(bool& task_executed) {
    ASSERT_FALSE(task_executed);
    EXPECT_CALL(*mock_message_pump_, GetNextPendingTask)
        .WillOnce(Return(ByMove(CreatePendingTask(base::BindOnce(
            [](bool* ext_task_executed) { *ext_task_executed = true; },
            &task_executed)))))
        .RetiresOnSaturation();
  }

  base::MessagePump::PendingTask CreateSequencedPendingTask(
      base::OnceClosure task,
      std::optional<base::SequenceId> sequence_id) {
    return {std::move(task), std::move(sequence_id), {}};
  }

  base::MessagePump::PendingTask CreatePendingTask(base::OnceClosure task) {
    return CreateSequencedPendingTask(std::move(task), {});
  }

  base::MessagePump::PendingTask CreateEmptyPendingTask() {
    return CreatePendingTask({});
  }

  base::MessagePump::PendingTask CreateCountingPendingTask(size_t& counter) {
    return CreatePendingTask(base::BindOnce(
        [](size_t* ext_counter) { (*ext_counter)++; }, &counter));
  }

 protected:
  std::shared_ptr<MockMessagePump> mock_message_pump_;
  std::unique_ptr<base::MessageLoopImpl> message_loop_impl_;
};

TEST_F(MessageLoopImplTest, RunOnceFailsWithoutTask) {
  SetupDefaultGetNextPendingTask();

  EXPECT_FALSE(message_loop_impl_->RunOnce());
  EXPECT_FALSE(message_loop_impl_->RunOnce());
}

TEST_F(MessageLoopImplTest, RunOnceExecutesSingleTask) {
  SetupDefaultGetNextPendingTask();
  EXPECT_FALSE(message_loop_impl_->RunOnce());

  bool task_executed = false;
  ExpectTaskExecution(task_executed);
  EXPECT_TRUE(message_loop_impl_->RunOnce());
  EXPECT_TRUE(task_executed);

  EXPECT_FALSE(message_loop_impl_->RunOnce());
}

TEST_F(MessageLoopImplTest, RunOnceExecutesOnlyOneTask) {
  bool task_executed = false;
  ExpectTaskExecution(task_executed);

  EXPECT_TRUE(message_loop_impl_->RunOnce());
}

TEST_F(MessageLoopImplTest, CurrentSequenceSetWhenRunning) {
  const auto sequence_id =
      base::detail::SequenceIdGenerator::GetNextSequenceId();
  bool was_current_sequence_set = false;
  bool was_task_runner_handle_set = false;

  auto task = CreateSequencedPendingTask(
      base::BindOnce(
          [](base::SequenceId target_seq_id, bool* flag1, bool* flag2) {
            ASSERT_NE(flag1, nullptr);
            ASSERT_NE(flag2, nullptr);
            if (base::detail::CurrentSequenceIdHelper::IsCurrentSequence(
                    target_seq_id)) {
              *flag1 = true;
              *flag2 = true;
            }
          },
          sequence_id, &was_current_sequence_set, &was_task_runner_handle_set),
      sequence_id);
  EXPECT_CALL(*mock_message_pump_, GetNextPendingTask)
      .WillOnce(Return(ByMove(std::move(task))));

  EXPECT_FALSE(
      base::detail::CurrentSequenceIdHelper::IsCurrentSequence(sequence_id));
  EXPECT_TRUE(message_loop_impl_->RunOnce());
  EXPECT_FALSE(
      base::detail::CurrentSequenceIdHelper::IsCurrentSequence(sequence_id));

  EXPECT_TRUE(was_current_sequence_set);
  EXPECT_TRUE(was_task_runner_handle_set);
}

TEST_F(MessageLoopImplTest, RunUntilIdleFinishesWithoutAnyTasks) {
  EXPECT_CALL(*mock_message_pump_, GetNextPendingTask)
      .WillOnce(Return(ByMove(CreateEmptyPendingTask())));
  message_loop_impl_->RunUntilIdle();
}

TEST_F(MessageLoopImplTest, RunUntilIdleExecutesAllPendingTasks) {
  size_t counter = 0;
  EXPECT_CALL(*mock_message_pump_, GetNextPendingTask)
      .WillOnce(Return(ByMove(CreateCountingPendingTask(counter))))
      .WillOnce(Return(ByMove(CreateCountingPendingTask(counter))))
      .WillOnce(Return(ByMove(CreateCountingPendingTask(counter))))
      .WillOnce(Return(ByMove(CreateEmptyPendingTask())));
  message_loop_impl_->RunUntilIdle();
  EXPECT_EQ(counter, 3u);
}

TEST_F(MessageLoopImplTest, StopIsForwarded) {
  EXPECT_CALL(*mock_message_pump_, Stop(_));
  message_loop_impl_->Stop(base::OnceClosure{});
}

TEST_F(MessageLoopImplTest, RunIsExecutedUntilStopIsCalled) {
  using namespace std::chrono_literals;

  EXPECT_CALL(*mock_message_pump_, Stop);
  EXPECT_CALL(*mock_message_pump_, GetNextPendingTask).WillRepeatedly([&]() {
    std::this_thread::sleep_for(10ms);
    return CreateEmptyPendingTask();
  });

  std::condition_variable cond_var;
  std::mutex mutex;

  bool run_started = false;
  std::atomic_bool run_finished = false;

  // Thread which executes tasks within message loop
  std::thread executor{[&]() {
    {
      std::lock_guard<std::mutex> lock(mutex);
      run_started = true;
    }
    cond_var.notify_one();
    message_loop_impl_->Run();
    run_finished = true;
  }};

  {
    std::unique_lock<std::mutex> lock(mutex);
    cond_var.wait(lock, [&run_started]() { return run_started; });
  }

  std::this_thread::sleep_for(30ms);
  EXPECT_FALSE(run_finished);

  message_loop_impl_->Stop(base::OnceClosure{});
  executor.join();
}

}  // namespace
