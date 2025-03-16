#include "base/init.h"

#include "base/logging.h"

namespace base {

void Initialize(int /*argc*/,
                char* argv[]) {  // NOLINT(modernize-avoid-c-arrays)
  FLAGS_logtostderr = true;

  google::InitGoogleLogging(argv[0]);
  google::InstallPrefixFormatter(&detail::LogFormatter);
  google::InstallFailureSignalHandler();
}

void Deinitialize() {
  google::ShutdownGoogleLogging();
}

}  // namespace base
