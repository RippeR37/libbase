#include "base/synchronization/auto_signaller.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"

#include "gtest/gtest.h"

namespace {

class AutoSignallerTest : public ::testing::Test {
 public:
  base::WaitableEvent event;
};

TEST_F(AutoSignallerTest, NoSignalOnCtor) {
  EXPECT_FALSE(event.IsSignaled());
  base::AutoSignaller signaller(&event);
  EXPECT_FALSE(event.IsSignaled());
}

TEST_F(AutoSignallerTest, SignalOnScopeExit) {
  EXPECT_FALSE(event.IsSignaled());
  {
    base::AutoSignaller signaller(&event);
    EXPECT_FALSE(event.IsSignaled());
  }
  EXPECT_TRUE(event.IsSignaled());
}

TEST_F(AutoSignallerTest, SignalAndReset) {
  EXPECT_FALSE(event.IsSignaled());
  base::AutoSignaller signaller(&event);
  signaller.SignalAndReset();
  EXPECT_TRUE(event.IsSignaled());
}

TEST_F(AutoSignallerTest, SignalAndResetOnlyOnce) {
  EXPECT_FALSE(event.IsSignaled());
  base::AutoSignaller signaller(&event);
  signaller.SignalAndReset();
  event.Reset();
  EXPECT_FALSE(event.IsSignaled());
  signaller.SignalAndReset();
  EXPECT_FALSE(event.IsSignaled());
}

TEST_F(AutoSignallerTest, Cancel) {
  EXPECT_FALSE(event.IsSignaled());
  {
    base::AutoSignaller signaller(&event);
    signaller.Cancel();
  }
  EXPECT_FALSE(event.IsSignaled());
}

TEST_F(AutoSignallerTest, SignalAndResetAfterCancel) {
  EXPECT_FALSE(event.IsSignaled());
  base::AutoSignaller signaller(&event);
  signaller.Cancel();
  signaller.SignalAndReset();
  EXPECT_FALSE(event.IsSignaled());
}

TEST_F(AutoSignallerTest, MoveCtor) {
  EXPECT_FALSE(event.IsSignaled());
  {
    auto signaller1 = std::make_unique<base::AutoSignaller>(&event);
    base::AutoSignaller signaller2 = std::move(*signaller1);
    EXPECT_FALSE(event.IsSignaled());
    signaller1->SignalAndReset();
    signaller1.reset();
    EXPECT_FALSE(event.IsSignaled());
  }
  EXPECT_TRUE(event.IsSignaled());
}

TEST_F(AutoSignallerTest, MoveAssignment) {
  EXPECT_FALSE(event.IsSignaled());
  {
    base::AutoSignaller signaller1(&event);
    signaller1.Cancel();  // cancel original signaller to avoid signalling on
                          // assigment
    EXPECT_FALSE(event.IsSignaled());
    {
      base::AutoSignaller signaller2(&event);
      EXPECT_FALSE(event.IsSignaled());
      signaller1 = std::move(signaller2);
      EXPECT_FALSE(event.IsSignaled());
    }
    EXPECT_FALSE(event.IsSignaled());
  }
  EXPECT_TRUE(event.IsSignaled());
}

TEST_F(AutoSignallerTest, SignalOnCallbackCompletion) {
  auto function = [&](base::AutoSignaller) {
    EXPECT_FALSE(event.IsSignaled());
  };

  EXPECT_FALSE(event.IsSignaled());
  function(base::AutoSignaller{&event});
  EXPECT_TRUE(event.IsSignaled());
}

TEST_F(AutoSignallerTest, SignalOnCallbackDestruction) {
  auto function = [](base::AutoSignaller) {};
  base::OnceClosure callback =
      base::BindOnce(function, base::AutoSignaller{&event});
  EXPECT_FALSE(event.IsSignaled());
  callback = base::OnceClosure{};
  EXPECT_TRUE(event.IsSignaled());
}

TEST_F(AutoSignallerTest, SignalOnPostTaskCompletion) {
  base::Thread thread;
  thread.Start();

  bool finished_flag = false;
  auto function = [](bool* flag, base::AutoSignaller) { *flag = true; };

  thread.TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(function, &finished_flag, &event));
  event.Wait();
  EXPECT_TRUE(finished_flag);
}

}  // namespace
