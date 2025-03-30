#include "base/net/impl/net_thread.h"

#include "base/logging.h"
#include "base/net/impl/net_thread_impl.h"

namespace base {
namespace net {

// static
NetThread& NetThread::GetInstance() {
  static NetThread instance;
  return instance;
}

void NetThread::Start() {
  DCHECK(!impl_);

  impl_ = std::make_unique<NetThreadImpl>();
  impl_->Start();
}

void NetThread::Stop() {
  DCHECK(impl_);

  impl_->Stop();
  impl_.reset();
}

void NetThread::EnqueueDownload(
    ResourceRequest request,
    std::optional<size_t> max_response_size_bytes,
    OnceCallback<void(ResourceResponse)> on_done_callback) {
  DCHECK(impl_);

  impl_->EnqueueDownload(std::move(request), std::move(max_response_size_bytes),
                         std::move(on_done_callback));
}

}  // namespace net
}  // namespace base
