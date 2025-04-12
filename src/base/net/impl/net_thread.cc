#include "base/net/impl/net_thread.h"

#include <atomic>

#include "base/logging.h"
#include "base/net/impl/net_thread_impl.h"

namespace base {
namespace net {

namespace {
std::atomic_size_t g_request_id_counter_ = 0;
}

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

RequestCancellationToken NetThread::EnqueueDownload(
    ResourceRequest request,
    std::optional<size_t> max_response_size_bytes,
    OnceCallback<void(ResourceResponse)> on_done_callback) {
  DCHECK(impl_);

  RequestCancellationToken cancellation_token{
      g_request_id_counter_.fetch_add(1)};

  impl_->EnqueueDownload(std::move(request), std::move(max_response_size_bytes),
                         cancellation_token, std::move(on_done_callback));

  return cancellation_token;
}

RequestCancellationToken NetThread::EnqueueDownload(
    ResourceRequest request,
    std::optional<size_t> max_response_size_bytes,
    OnceCallback<void(int, std::string, std::map<std::string, std::string>)>
        on_response_started,
    RepeatingCallback<void(std::vector<uint8_t>)> on_write_data,
    OnceCallback<void(Result)> on_finished) {
  DCHECK(impl_);

  RequestCancellationToken cancellation_token{
      g_request_id_counter_.fetch_add(1)};

  impl_->EnqueueDownload(std::move(request), std::move(max_response_size_bytes),
                         cancellation_token, std::move(on_response_started),
                         std::move(on_write_data), std::move(on_finished));

  return cancellation_token;
}

void NetThread::CancelRequest(RequestCancellationToken cancellation_token) {
  DCHECK(impl_);

  impl_->CancelRequest(std::move(cancellation_token));
}

}  // namespace net
}  // namespace base
