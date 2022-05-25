#include "base/threading/delayed_task_manager_shared_instance.h"

#include "gtest/gtest.h"

namespace {

TEST(DelayedTaskManagerSharedInstanceTest, SameInstance) {
  auto result1 =
      base::DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance();
  auto result2 =
      base::DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance();

  EXPECT_EQ(result1.get(), result2.get());
}

}  // namespace
