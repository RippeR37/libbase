#include "base/message_loop/run_loop.h"

#include "gtest/gtest.h"

namespace {

using ::testing::Test;

void SetFlag(bool* flag) {
  *flag = true;
}

class RunLoopTest : public Test {
 public:
  void SetUp() override { run_loop_ = std::make_unique<base::RunLoop>(); }

  void TearDown() override { run_loop_.reset(); }

 protected:
  std::unique_ptr<base::RunLoop> run_loop_;
};

TEST_F(RunLoopTest, Noop) {
  // Do nothing
}

TEST_F(RunLoopTest, Quit_Empty) {
  run_loop_->Quit();
  run_loop_->Quit();
}

TEST_F(RunLoopTest, RunOnce) {
  bool first_task_executed = false;
  bool second_task_executed = false;

  run_loop_->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&SetFlag, &first_task_executed));
  run_loop_->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&SetFlag, &second_task_executed));

  EXPECT_FALSE(first_task_executed);
  EXPECT_FALSE(second_task_executed);
  run_loop_->RunOnce();
  EXPECT_TRUE(first_task_executed);
  EXPECT_FALSE(second_task_executed);
  run_loop_->Quit();
  run_loop_->RunOnce();
  EXPECT_TRUE(first_task_executed);
  EXPECT_TRUE(second_task_executed);
}

TEST_F(RunLoopTest, RunUntilIdle_Empty) {
  // This should be no-op
  run_loop_->RunUntilIdle();
}

TEST_F(RunLoopTest, RunUntilIdle_MultipleTasks) {
  bool first_task_executed = false;
  bool second_task_executed = false;
  bool posted_task_executed = false;
  bool after_quit_task_executed = false;

  run_loop_->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&SetFlag, &first_task_executed));
  run_loop_->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(
                     [](std::shared_ptr<base::SequencedTaskRunner> task_runner,
                        bool* posted_task_executed_ptr) {
                       task_runner->PostTask(
                           FROM_HERE,
                           base::BindOnce(&SetFlag, posted_task_executed_ptr));
                     },
                     run_loop_->TaskRunner(), &posted_task_executed));
  run_loop_->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&SetFlag, &second_task_executed));

  EXPECT_FALSE(first_task_executed);
  EXPECT_FALSE(second_task_executed);
  EXPECT_FALSE(posted_task_executed);
  EXPECT_FALSE(after_quit_task_executed);

  run_loop_->RunUntilIdle();
  EXPECT_TRUE(first_task_executed);
  EXPECT_TRUE(second_task_executed);
  EXPECT_TRUE(posted_task_executed);
  EXPECT_FALSE(after_quit_task_executed);

  run_loop_->Quit();
  run_loop_->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&SetFlag, &after_quit_task_executed));

  run_loop_->RunUntilIdle();
  EXPECT_TRUE(first_task_executed);
  EXPECT_TRUE(second_task_executed);
  EXPECT_TRUE(posted_task_executed);
  EXPECT_FALSE(after_quit_task_executed);
}

TEST_F(RunLoopTest, Run_Simple) {
  bool first_task_executed = false;
  bool second_task_executed = false;

  run_loop_->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&SetFlag, &first_task_executed));
  run_loop_->TaskRunner()->PostTask(FROM_HERE, run_loop_->QuitClosure());
  run_loop_->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&SetFlag, &second_task_executed));

  EXPECT_FALSE(first_task_executed);
  EXPECT_FALSE(second_task_executed);
  run_loop_->Run();
  EXPECT_TRUE(first_task_executed);
  EXPECT_TRUE(second_task_executed);
}

TEST_F(RunLoopTest, Run_Advanced) {
  struct Flags {
    bool first_executed = false;
    bool quit_called = false;
    bool quit_cb_executed = false;
    bool run_finished = false;

    bool failed_checks = false;
  };

  Flags flags;

  run_loop_->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&SetFlag, &flags.first_executed));
  run_loop_->TaskRunner()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(
          [](Flags* flags_ptr,
             base::OnceCallback<void(base::OnceClosure)> quit_cb) {
            if (!flags_ptr->first_executed || flags_ptr->quit_called ||
                flags_ptr->quit_cb_executed || flags_ptr->run_finished) {
              flags_ptr->failed_checks = true;
            }
            flags_ptr->quit_called = true;
            std::move(quit_cb).Run(
                base::BindOnce(&SetFlag, &flags_ptr->quit_cb_executed));
          },
          &flags, run_loop_->QuitCallback()),
      base::Milliseconds(150));

  EXPECT_FALSE(flags.first_executed);
  EXPECT_FALSE(flags.quit_called);
  EXPECT_FALSE(flags.quit_cb_executed);
  EXPECT_FALSE(flags.run_finished);
  EXPECT_FALSE(flags.failed_checks);
  run_loop_->Run();

  EXPECT_TRUE(flags.first_executed);
  EXPECT_TRUE(flags.quit_called);
  EXPECT_TRUE(flags.quit_cb_executed);
  EXPECT_FALSE(flags.run_finished);
  EXPECT_FALSE(flags.failed_checks);

  flags.run_finished = true;

  bool can_still_run_once_or_until_idle = false;
  run_loop_->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&SetFlag, &can_still_run_once_or_until_idle));
  run_loop_->RunUntilIdle();
  EXPECT_FALSE(flags.failed_checks);
  EXPECT_FALSE(can_still_run_once_or_until_idle);
}

}  // namespace
