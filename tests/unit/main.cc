#include "base/init.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
  base::Initialize(argc, argv);

  ::testing::InitGoogleTest(&argc, argv);
  const auto result = RUN_ALL_TESTS();

  base::Deinitialize();
  return result;
}
