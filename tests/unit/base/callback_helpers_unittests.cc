#include "base/callback_helpers.h"

#include "gtest/gtest.h"

namespace {

void SetFlag(bool* flag) {
  ASSERT_EQ(*flag, false);
  *flag = true;
}

TEST(ScopedClosureRunnerTest, Simple) {
  {
    base::ScopedClosureRunner empty;
    EXPECT_FALSE(empty);
    // Nothing should happen here
  }

  bool flag = false;
  {
    base::ScopedClosureRunner scoped_closure{base::BindOnce(&SetFlag, &flag)};
    // Should set the |flag| after exiting this scope
  }
  EXPECT_TRUE(flag);
}

TEST(ScopedClosureRunnerTest, MoveCtorBehavior) {
  bool flag = false;
  {
    base::ScopedClosureRunner scoped_closure{base::BindOnce(&SetFlag, &flag)};
    {
      base::ScopedClosureRunner inner_scoped_closure =
          std::move(scoped_closure);
    }
    EXPECT_FALSE(scoped_closure);
    EXPECT_TRUE(flag);
  }
}

TEST(ScopedClosureRunnerTest, RunPreviousOnAssignment) {
  bool flag1 = false;
  bool flag2 = false;

  {
    base::ScopedClosureRunner scoped_closure{base::BindOnce(&SetFlag, &flag1)};
    EXPECT_FALSE(flag1);
    EXPECT_FALSE(flag2);
    EXPECT_TRUE(scoped_closure);

    scoped_closure =
        base::ScopedClosureRunner{base::BindOnce(&SetFlag, &flag2)};
    EXPECT_TRUE(flag1);
    EXPECT_FALSE(flag2);
    EXPECT_TRUE(scoped_closure);
  }

  EXPECT_TRUE(flag1);
  EXPECT_TRUE(flag2);
}

TEST(ScopedClosureRunnerTest, RunAndReset) {
  bool flag = false;
  {
    base::ScopedClosureRunner scoped_closure{base::BindOnce(&SetFlag, &flag)};
    scoped_closure.RunAndReset();
    EXPECT_TRUE(flag);
    EXPECT_FALSE(scoped_closure);
  }
}

TEST(ScopedClosureRunnerTest, ReplaceClosure) {
  bool flag1 = false;
  bool flag2 = false;

  {
    base::ScopedClosureRunner scoped_closure{base::BindOnce(&SetFlag, &flag1)};
    scoped_closure.ReplaceClosure(base::BindOnce(&SetFlag, &flag2));
    EXPECT_TRUE(scoped_closure);
  }
  EXPECT_FALSE(flag1);
  EXPECT_TRUE(flag2);
}

TEST(ScopedClosureRunnerTest, Release) {
  bool flag = false;
  base::OnceClosure closure;

  {
    base::ScopedClosureRunner scoped_closure{base::BindOnce(&SetFlag, &flag)};
    EXPECT_TRUE(scoped_closure);
    closure = scoped_closure.Release();
    EXPECT_FALSE(scoped_closure);
  }

  EXPECT_FALSE(flag);
  std::move(closure).Run();
  EXPECT_TRUE(flag);
}

}  // namespace
