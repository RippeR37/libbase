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
  int task_runner_matched = false;
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

}  // namespace
