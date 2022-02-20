#include "base/sequenced_task_runner.h"

#include "base/bind.h"
#include "base/synchronization/auto_signaller.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"

#include "gtest/gtest.h"

namespace {

class DeleteNotifier {
 public:
  DeleteNotifier(bool* flag,
                 std::shared_ptr<base::SequencedTaskRunner> delete_task_runner)
      : flag_(flag), delete_task_runner_(std::move(delete_task_runner)) {}

  ~DeleteNotifier() {
    EXPECT_FALSE(*flag_);
    *flag_ = true;

    if (delete_task_runner_) {
      EXPECT_TRUE(delete_task_runner_->RunsTasksInCurrentSequence());
    }
  }

  void CleanupInTest() {
    // This is to cleanup resources after test finishes
    delete_task_runner_.reset();
    delete this;
  }

 private:
  bool* flag_;
  std::shared_ptr<base::SequencedTaskRunner> delete_task_runner_;
};

class SequencedTaskRunnerTest : public ::testing::Test {
 public:
  void SetUp() override {
    thread = std::make_unique<base::Thread>();
    thread->Start();
    task_runner = thread->TaskRunner();
  }

  void TearDown() override {
    task_runner.reset();
    thread.reset();
  }

  std::unique_ptr<base::Thread> thread;
  std::shared_ptr<base::SequencedTaskRunner> task_runner;
};

TEST_F(SequencedTaskRunnerTest, DeleteSoonRawPtr) {
  bool object_deleted = false;
  auto* object = new DeleteNotifier(&object_deleted, task_runner);
  EXPECT_TRUE(task_runner->DeleteSoon(FROM_HERE, object));
  thread->FlushForTesting();
  EXPECT_TRUE(object_deleted);
}

TEST_F(SequencedTaskRunnerTest, DeleteSoonUniquePtr) {
  bool object_deleted = false;
  auto object = std::make_unique<DeleteNotifier>(&object_deleted, task_runner);
  EXPECT_TRUE(task_runner->DeleteSoon(FROM_HERE, std::move(object)));
  thread->FlushForTesting();
  EXPECT_TRUE(object_deleted);
}

TEST_F(SequencedTaskRunnerTest, LeakRawIfFailed) {
  bool object_deleted = false;
  auto* object = new DeleteNotifier(&object_deleted, task_runner);
  thread->Join();

  EXPECT_FALSE(task_runner->DeleteSoon(FROM_HERE, object));
  EXPECT_FALSE(object_deleted);

  object->CleanupInTest();
}

TEST_F(SequencedTaskRunnerTest, LeakUniqueIfFailed) {
  bool object_deleted = false;
  auto object = std::make_unique<DeleteNotifier>(&object_deleted, task_runner);
  auto* object_ptr = object.get();
  thread->Join();

  EXPECT_FALSE(task_runner->DeleteSoon(FROM_HERE, std::move(object)));
  EXPECT_FALSE(object_deleted);

  object_ptr->CleanupInTest();
}

}  // namespace
