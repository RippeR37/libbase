#include "base/bind.h"

#include "gtest/gtest.h"

namespace {

int add(int x, int y) {
  return x + y;
}

}  // namespace

TEST(BindOnceTest, Simple) {
  auto add_callback = base::BindOnce(&add);
  EXPECT_EQ(5, std::move(add_callback).Run(2, 3));

  auto increment_callback = base::BindOnce(&add, 1);
  EXPECT_EQ(7, std::move(increment_callback).Run(6));
}
