#include "base/init.h"

#include "base/logging.h"

namespace base {

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void Initialize(int /*argc*/, char* argv[]) {
  FLAGS_logtostderr = true;

  google::InitGoogleLogging(argv[0]);
  google::InstallPrefixFormatter(&detail::LogFormatter);
  google::InstallFailureSignalHandler();
}

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void InitializeForTests(int /*argc*/, char* argv[]) {
  FLAGS_logtostderr = true;

  google::InitGoogleLogging(argv[0]);
  google::InstallPrefixFormatter(&detail::LogFormatter);
}

void Deinitialize() {
  google::ShutdownGoogleLogging();
}

}  // namespace base
