#include "base/init.h"

#include "base/logging.h"
#include "base/net/impl/net_thread.h"

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

  if (g_init_options.InitializeNetworking) {
    // Initialize Networking thread
    net::NetThread::GetInstance().Start();
  }
}

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void InitializeForTests(int /*argc*/, char* argv[], InitOptions options) {
  g_init_options = options;

  FLAGS_logtostderr = g_init_options.LogToStderr;

  google::InitGoogleLogging(argv[0]);
  google::InstallPrefixFormatter(&detail::LogFormatter);

  if (g_init_options.InitializeNetworking) {
    // Initialize Networking thread
    net::NetThread::GetInstance().Start();
  }
}

void Deinitialize() {
  if (g_init_options.InitializeNetworking) {
    // Deinitialize Networking thread
    net::NetThread::GetInstance().Stop();
  }

  google::ShutdownGoogleLogging();
}

}  // namespace base
