#include "base/threading/delayed_task_manager.h"

#include <atomic>
#include <memory>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/message_loop/message_pump.h"
#include "base/message_loop/mock_message_pump.h"
#include "base/synchronization/auto_signaller.h"
#include "base/synchronization/waitable_event.h"

#include "gtest/gtest.h"

namespace {

const base::TimeDelta kZero = base::TimeDelta{};

std::atomic<base::TimeTicks> g_now_mocked;

base::TimeTicks GetMockedTimeTicks() {
  return g_now_mocked;
}

base::TimeTicks AsTimeTicks(base::TimeDelta delta) {
  return base::TimeTicks{} + delta;
}

void SetMockedTimeTicks(base::TimeDelta delta) {
  g_now_mocked = base::TimeTicks{} + delta;
}

void SetMockedTimeTicks(base::TimeTicks ticks) {
  g_now_mocked = ticks;
}

base::MessagePump::PendingTask GetPendingTask(base::OnceClosure task) {
  return {std::move(task), {}, {}, {}};
}

base::MessagePump::PendingTask GetEmptyPendingTask() {
  return GetPendingTask(base::DoNothing{});
}

base::MessagePump::PendingTask GetSignalingPendingTask(
    base::WaitableEvent* event) {
  return GetPendingTask(
      base::BindOnce([](base::AutoSignaller) {}, base::AutoSignaller{event}));
}

bool ExecutePendingTask(base::MessagePump::PendingTask pending_task) {
  std::move(pending_task.task).Run();
  return true;
}

class DelayedTaskManagerTest : public ::testing::Test {
 public:
  void SetUp() override {
    SetMockedTimeTicks(base::TimeTicks{});

    dtm = std::make_unique<base::DelayedTaskManager>(GetTimeTicksProvider());
    mock_message_pump_ = std::make_unique<MockMessagePump>();

    ON_CALL(*mock_message_pump_, QueuePendingTask)
        .WillByDefault(&ExecutePendingTask);
  }

  void TearDown() override {
    mock_message_pump_.reset();
    dtm.reset();
  }

  virtual base::DelayedTaskManager::TimeTicksProvider GetTimeTicksProvider() {
    return &GetMockedTimeTicks;
  }

  std::unique_ptr<base::DelayedTaskManager> dtm;
  std::shared_ptr<MockMessagePump> mock_message_pump_;
};

TEST_F(DelayedTaskManagerTest, QueueImmediatelyAtTaskTime) {
  EXPECT_CALL(*mock_message_pump_, QueuePendingTask);
  dtm->QueueDelayedTask(base::DelayedTaskManager::DelayedTask{
      AsTimeTicks(kZero), mock_message_pump_, GetEmptyPendingTask()});
}

TEST_F(DelayedTaskManagerTest, QueueImmediatelyAfterTaskTime) {
  SetMockedTimeTicks(base::Milliseconds(10));

  EXPECT_CALL(*mock_message_pump_, QueuePendingTask);
  dtm->QueueDelayedTask(base::DelayedTaskManager::DelayedTask{
      AsTimeTicks(kZero), mock_message_pump_, GetEmptyPendingTask()});
}

TEST_F(DelayedTaskManagerTest, DoesNotQueueBeforeTaskTime) {
  EXPECT_CALL(*mock_message_pump_, QueuePendingTask).Times(0);
  dtm->QueueDelayedTask(base::DelayedTaskManager::DelayedTask{
      AsTimeTicks(base::Milliseconds(1)), mock_message_pump_,
      GetEmptyPendingTask()});

  // Won't schedule even after invoking it directly
  dtm->ScheduleAllReadyTasksForTests();
}

TEST_F(DelayedTaskManagerTest, QueueAfterTimeProgresses) {
  EXPECT_CALL(*mock_message_pump_, QueuePendingTask).Times(0);
  dtm->QueueDelayedTask(base::DelayedTaskManager::DelayedTask{
      AsTimeTicks(base::Seconds(1)), mock_message_pump_,
      GetEmptyPendingTask()});

  // Won't schedule even after invoking it directly
  dtm->ScheduleAllReadyTasksForTests();

  ::testing::Mock::VerifyAndClearExpectations(mock_message_pump_.get());

  EXPECT_CALL(*mock_message_pump_, QueuePendingTask);
  SetMockedTimeTicks(base::Seconds(1));
  dtm->ScheduleAllReadyTasksForTests();
}

//
//
//

std::atomic<bool> g_allow_time_to_progress = true;

base::TimeTicks GetAndIncrementMockedTimeTicks() {
  const auto result = GetMockedTimeTicks();
  if (g_allow_time_to_progress) {
    // Increment time for next query
    SetMockedTimeTicks(result + base::Microseconds(1));
  }
  return result;
}

class DelayedTaskManagerProgressingTimeTest : public DelayedTaskManagerTest {
 public:
  base::DelayedTaskManager::TimeTicksProvider GetTimeTicksProvider() override {
    return &GetAndIncrementMockedTimeTicks;
  }
};

TEST_F(DelayedTaskManagerProgressingTimeTest,
       QueueAfterTimeProgressesOnItsOwn) {
  base::WaitableEvent on_queue_called;

  EXPECT_CALL(*mock_message_pump_, QueuePendingTask);
  dtm->QueueDelayedTask(base::DelayedTaskManager::DelayedTask{
      AsTimeTicks(base::Microseconds(3)), mock_message_pump_,
      GetSignalingPendingTask(&on_queue_called)});

  on_queue_called.Wait();
}

TEST_F(DelayedTaskManagerProgressingTimeTest, QueueInCorrectOrder) {
  base::WaitableEvent on_finish;
  std::atomic<bool> later_task_executed = false;

  g_allow_time_to_progress = false;

  EXPECT_CALL(*mock_message_pump_, QueuePendingTask).Times(2);
  dtm->QueueDelayedTask(base::DelayedTaskManager::DelayedTask{
      AsTimeTicks(base::Microseconds(6)), mock_message_pump_,
      GetPendingTask(base::BindOnce(
          [](std::atomic<bool>* flag, base::AutoSignaller) { *flag = true; },
          &later_task_executed, base::AutoSignaller{&on_finish}))});

  // This one is expected to be queued first
  dtm->QueueDelayedTask(base::DelayedTaskManager::DelayedTask{
      AsTimeTicks(base::Microseconds(3)), mock_message_pump_,
      GetPendingTask(
          base::BindOnce([](std::atomic<bool>* flag) { EXPECT_FALSE(*flag); },
                         &later_task_executed))});

  g_allow_time_to_progress = true;
  on_finish.Wait();

  EXPECT_TRUE(later_task_executed);
}

}  // namespace
