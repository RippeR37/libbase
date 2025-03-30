#include "base/bind_post_task.h"

#include "base/threading/thread.h"

#include "gtest/gtest.h"

namespace {

void SetFlagIfRunningOnTaskRunner(
    bool* executed_flag,
    bool* flag,
    std::shared_ptr<base::SequencedTaskRunner> task_runner) {
  *executed_flag = true;
  *flag = task_runner->RunsTasksInCurrentSequence();
}

void IncrementIfRunningOnTaskRunner(
    int* executed_counter,
    int* counter,
    std::shared_ptr<base::SequencedTaskRunner> task_runner) {
  *executed_counter += 1;
  *counter += task_runner->RunsTasksInCurrentSequence() ? 1 : 0;
}

class BindPostTaskTest : public ::testing::Test {
 public:
  void SetUp() override { thread.Start(); }
  void TearDown() override { thread.Stop(); }

  base::Thread thread;
};

TEST_F(BindPostTaskTest, BindOnce) {
  bool executed = false;
  bool task_runner_matched = false;
  auto callback =
      base::BindPostTask(thread.TaskRunner(),
                         base::BindOnce(&SetFlagIfRunningOnTaskRunner,
                                        &executed, &task_runner_matched),
                         FROM_HERE);
  thread.FlushForTesting();
  EXPECT_FALSE(executed);
  EXPECT_FALSE(task_runner_matched);
  std::move(callback).Run(thread.TaskRunner());
  thread.FlushForTesting();
  EXPECT_TRUE(executed);
  EXPECT_TRUE(task_runner_matched);
}

TEST_F(BindPostTaskTest, BindRepeating) {
  int executed = false;
  int task_runner_matched = 0;
  auto callback =
      base::BindPostTask(thread.TaskRunner(),
                         base::BindRepeating(&IncrementIfRunningOnTaskRunner,
                                             &executed, &task_runner_matched),
                         FROM_HERE);
  thread.FlushForTesting();
  EXPECT_EQ(executed, 0);
  EXPECT_EQ(task_runner_matched, 0);

  callback.Run(thread.TaskRunner());
  thread.FlushForTesting();
  EXPECT_EQ(executed, 1);
  EXPECT_EQ(task_runner_matched, 1);

  std::move(callback).Run(thread.TaskRunner());
  thread.FlushForTesting();
  EXPECT_EQ(executed, 2);
  EXPECT_EQ(task_runner_matched, 2);
}

class BindToCurrentSequenceTest : public BindPostTaskTest {
 public:
  void SetUp() override {
    BindPostTaskTest::SetUp();
    thread2.Start();
  }

  void TearDown() override {
    BindPostTaskTest::TearDown();
    thread2.Stop();
  }

  base::Thread thread2;
};

TEST_F(BindToCurrentSequenceTest, Once) {
  bool executed = false;
  bool task_runner_matched = false;

  thread.TaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](bool* executed_ptr, bool* task_runner_matched_ptr,
             std::shared_ptr<base::SequencedTaskRunner> expected_task_runner,
             std::shared_ptr<base::SequencedTaskRunner> post_to_task_runner) {
            auto callback = base::BindToCurrentSequence(
                base::BindOnce(&SetFlagIfRunningOnTaskRunner, executed_ptr,
                               task_runner_matched_ptr, expected_task_runner),
                FROM_HERE);
            post_to_task_runner->PostTask(FROM_HERE, std::move(callback));
          },
          &executed, &task_runner_matched, thread.TaskRunner(),
          thread2.TaskRunner()));

  // Ensure first task posting to thread2 executed
  thread.FlushForTesting();
  // Ensure second task executing bound callback on thread 2 executed
  thread2.FlushForTesting();
  // Ensure bound-post-tasked callback executed on thread 1
  thread.FlushForTesting();

  EXPECT_TRUE(executed);
  EXPECT_TRUE(task_runner_matched);
}

TEST_F(BindToCurrentSequenceTest, Repeating) {
  int executed = 0;
  int task_runner_matched = 0;

  thread.TaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](int* executed_ptr, int* task_runner_matched_ptr,
             std::shared_ptr<base::SequencedTaskRunner> expected_task_runner,
             std::shared_ptr<base::SequencedTaskRunner> post_to_task_runner) {
            auto callback = base::BindToCurrentSequence(
                base::BindRepeating(&IncrementIfRunningOnTaskRunner,
                                    executed_ptr, task_runner_matched_ptr,
                                    expected_task_runner),
                FROM_HERE);

            post_to_task_runner->PostTask(FROM_HERE, callback);
            post_to_task_runner->PostTask(FROM_HERE, callback);
            post_to_task_runner->PostTask(FROM_HERE, callback);
          },
          &executed, &task_runner_matched, thread.TaskRunner(),
          thread2.TaskRunner()));

  // Ensure first task posting to thread2 executed
  thread.FlushForTesting();
  // Ensure second task executing bound callback on thread 2 executed
  thread2.FlushForTesting();
  // Ensure bound-post-tasked callback executed on thread 1
  thread.FlushForTesting();

  EXPECT_EQ(executed, 3);
  EXPECT_EQ(task_runner_matched, 3);
}

}  // namespace
