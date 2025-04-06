#include "base/net/init.h"

#include "base/net/impl/net_thread.h"

namespace base {
namespace net {

namespace {
InitOptions g_net_init_options;
}

void Initialize(InitOptions options) {
  g_net_init_options = options;

  net::NetThread::GetInstance().Start();
}

void Deinitialize() {
  net::NetThread::GetInstance().Stop();
}

}  // namespace net
}  // namespace base
