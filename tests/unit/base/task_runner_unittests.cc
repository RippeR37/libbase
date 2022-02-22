#include "base/task_runner.h"

#include "base/bind.h"
#include "base/synchronization/auto_signaller.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"

#include "gtest/gtest.h"

namespace {

class TaskRunnerTest : public ::testing::Test {
 public:
  void SetUp() override {
    thread1 = std::make_unique<base::Thread>();
    thread2 = std::make_unique<base::Thread>();

    thread1->Start();
    thread2->Start();
  }

  void TearDown() override {
    thread1.reset();
    thread2.reset();
  }

  std::shared_ptr<base::SequencedTaskRunner> TaskRunner1() {
    return thread1->TaskRunner();
  }

  std::shared_ptr<base::SequencedTaskRunner> TaskRunner2() {
    return thread2->TaskRunner();
  }

  std::unique_ptr<base::Thread> thread1;
  std::unique_ptr<base::Thread> thread2;
};

class TaskRunnerPostTaskAndReplyTest : public TaskRunnerTest {
 public:
  void Task1(base::AutoSignaller guard) {
    EXPECT_TRUE(TaskRunner1()->RunsTasksInCurrentSequence());
    base::WaitableEvent end_of_task1{};
    TaskRunner2()->PostTaskAndReply(
        FROM_HERE,
        base::BindOnce(&TaskRunnerPostTaskAndReplyTest::Task2,
                       base::Unretained(this), &end_of_task1),
        base::BindOnce(&TaskRunnerPostTaskAndReplyTest::Task3,
                       base::Unretained(this), std::move(guard)));
    end_of_task1.Wait();
  }

  void Task2(base::WaitableEvent* signal_end_of_task1) {
    EXPECT_TRUE(TaskRunner2()->RunsTasksInCurrentSequence());
    ASSERT_TRUE(signal_end_of_task1);
    task2_completed = true;
    signal_end_of_task1->Signal();
  }

  void Task3(base::AutoSignaller) {
    EXPECT_TRUE(TaskRunner1()->RunsTasksInCurrentSequence());
    task3_completed = true;
  }

  bool task2_completed = false;
  bool task3_completed = false;
};

TEST_F(TaskRunnerPostTaskAndReplyTest, PostTaskAndReply) {
  base::WaitableEvent finished_event{};
  TaskRunner1()->PostTask(FROM_HERE,
                          base::BindOnce(&TaskRunnerPostTaskAndReplyTest::Task1,
                                         base::Unretained(this),
                                         base::AutoSignaller{&finished_event}));
  finished_event.Wait();
  EXPECT_TRUE(task2_completed);
  EXPECT_TRUE(task3_completed);
}

class TaskRunnerPostTaskAndReplyWithResultTest : public TaskRunnerTest {
 public:
  void Task1(base::AutoSignaller guard, int n) {
    EXPECT_TRUE(TaskRunner1()->RunsTasksInCurrentSequence());
    TaskRunner2()->PostTaskAndReplyWithResult(
        FROM_HERE,
        base::BindOnce(&TaskRunnerPostTaskAndReplyWithResultTest::Task2,
                       base::Unretained(this), n),
        base::BindOnce(&TaskRunnerPostTaskAndReplyWithResultTest::Task3,
                       base::Unretained(this), std::move(guard)));
  }

  int Task2(int n) {
    EXPECT_TRUE(TaskRunner2()->RunsTasksInCurrentSequence());
    return (n / 2);
  }

  void Task3(base::AutoSignaller, int n) {
    EXPECT_TRUE(TaskRunner1()->RunsTasksInCurrentSequence());
    task3_result = n;
  }

  std::optional<int> task3_result;
};

TEST_F(TaskRunnerPostTaskAndReplyWithResultTest, PostTaskAndReplyWithResult) {
  base::WaitableEvent finished_event{};
  TaskRunner1()->PostTask(
      FROM_HERE,
      base::BindOnce(&TaskRunnerPostTaskAndReplyWithResultTest::Task1,
                     base::Unretained(this),
                     base::AutoSignaller{&finished_event}, 7));
  finished_event.Wait();
  ASSERT_TRUE(task3_result);
  EXPECT_EQ(*task3_result, (7 / 2));
}

}  // namespace
