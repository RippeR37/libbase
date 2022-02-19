#include "base/threading/thread.h"

#include <memory>

#include "base/bind.h"
#include "base/synchronization/waitable_event.h"

#include "gtest/gtest.h"

namespace {

class ThreadTest : public ::testing::Test {
 public:
  void SetUp() override { thread = std::make_unique<base::Thread>(); }
  void TearDown() override { thread.reset(); }

  std::unique_ptr<base::Thread> thread;
};

TEST_F(ThreadTest, MultipleStartJoin) {
  thread->Start();
  thread->Join();
  thread->Join();
  thread->Join();
  thread->Start();
  thread->Join();
}

TEST_F(ThreadTest, TaskRunnerOnlyWhenRunning) {
  EXPECT_EQ(thread->TaskRunner(), nullptr);
  thread->Start();
  EXPECT_NE(thread->TaskRunner(), nullptr);
  thread->Join();
  EXPECT_EQ(thread->TaskRunner(), nullptr);
}

TEST_F(ThreadTest, AllQueuedTasksAreExecuted) {
  thread->Start();

  base::WaitableEvent first_task_started_event{};
  base::WaitableEvent ready_to_end_first_task_event{};
  auto first_task = [](base::WaitableEvent* task_started_event,
                       base::WaitableEvent* ready_to_wait_event) {
    task_started_event->Signal();
    ready_to_wait_event->Wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  };

  thread->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(first_task, &first_task_started_event,
                                &ready_to_end_first_task_event));
  first_task_started_event.Wait();

  bool second_task_executed = false;
  auto second_task = [](bool* executed_flag) { *executed_flag = true; };

  thread->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(second_task, &second_task_executed));
  ready_to_end_first_task_event.Signal();

  thread->Join();
  EXPECT_TRUE(second_task_executed);
}

}  // namespace
