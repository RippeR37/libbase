#include "base/init.h"

#include "base/logging.h"

namespace base {

namespace {
InitOptions g_init_options;
}

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void Initialize(int /*argc*/, char* argv[], InitOptions options) {
  g_init_options = options;

  FLAGS_logtostderr = g_init_options.LogToStderr;

  google::InitGoogleLogging(argv[0]);
  google::InstallPrefixFormatter(&detail::LogFormatter);
  google::InstallFailureSignalHandler();
}

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void InitializeForTests(int /*argc*/, char* argv[], InitOptions options) {
  g_init_options = options;

  FLAGS_logtostderr = g_init_options.LogToStderr;

  google::InitGoogleLogging(argv[0]);
  google::InstallPrefixFormatter(&detail::LogFormatter);
}

void Deinitialize() {
  google::ShutdownGoogleLogging();
}

}  // namespace base
