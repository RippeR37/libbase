#include "base/net/url_request.h"

#include "base/bind_post_task.h"
#include "base/net/impl/net_thread.h"

namespace base {
namespace net {

UrlRequest::UrlRequest(Client* client) : weak_factory_(client) {}

UrlRequest::~UrlRequest() {
  Cancel();
}

void UrlRequest::Start(ResourceRequest request) {
  CHECK(!cancellation_token_);

  weak_factory_.InvalidateWeakPtrs();
  weak_client_ = weak_factory_.GetWeakPtr();

  // When `OnRequestFinished` call is performed, we need to clean up the
  // cancellation token so that the user can start another request immediately.
  // We can avoid having separate weak pointers for Client and this as both will
  // have the exact same "lifetime".
  auto on_request_finished_and_cleanup = [](base::WeakPtr<Client> weak_client,
                                            UrlRequest* this_ptr,
                                            Result result) {
    if (weak_client) {
      this_ptr->cancellation_token_.reset();
      weak_client->OnRequestFinished(this_ptr, result);
    }
  };

  cancellation_token_ = NetThread::GetInstance().EnqueueDownload(
      std::move(request), std::nullopt,
      BindToCurrentSequence(
          BindOnce(&Client::OnResponseStarted, weak_client_, this), FROM_HERE),
      BindToCurrentSequence(
          BindRepeating(&Client::OnWriteData, weak_client_, this), FROM_HERE),
      BindToCurrentSequence(
          base::BindOnce(on_request_finished_and_cleanup, weak_client_, this),
          FROM_HERE));
}

void UrlRequest::Cancel() {
  weak_factory_.InvalidateWeakPtrs();

  if (!cancellation_token_) {
    return;
  }

  NetThread::GetInstance().CancelRequest(std::move(*cancellation_token_));
  cancellation_token_.reset();
}

}  // namespace net
}  // namespace base
