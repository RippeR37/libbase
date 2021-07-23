#include "base/auto_reset.h"

#include <memory>

#include "gtest/gtest.h"

namespace {

TEST(AutoResetTest, Simple) {
  int value = 1;

  {
    base::AutoReset<int> ar(&value, 2);
    EXPECT_EQ(value, 2);
  }
  EXPECT_EQ(value, 1);
}

TEST(AutoResetTest, MoveOnly) {
  auto value = std::make_unique<int>(1);
  const auto original_value_ptr = value.get();

  {
    auto new_value = std::make_unique<int>(2);
    const auto* new_value_ptr = new_value.get();

    base::AutoReset<std::unique_ptr<int>> ar(&value, std::move(new_value));
    EXPECT_EQ(value.get(), new_value_ptr);
  }
  EXPECT_EQ(value.get(), original_value_ptr);
}

}  // namespace
