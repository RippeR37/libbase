#include "base/logging.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
  FLAGS_logtostderr = true;
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  ::testing::InitGoogleTest(&argc, argv);
  const auto result = RUN_ALL_TESTS();

  google::ShutdownGoogleLogging();
  return result;
}
