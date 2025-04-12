#include "base/net/url_request.h"

#include "base/bind_post_task.h"
#include "base/net/impl/net_thread.h"

namespace base {
namespace net {

UrlRequest::UrlRequest(Client* client) : weak_factory_(client) {
  weak_client_ = weak_factory_.GetWeakPtr();
}

UrlRequest::~UrlRequest() {
  Cancel();
}

void UrlRequest::Download(ResourceRequest request) {
  DCHECK(weak_client_);

  cancellation_token_ = NetThread::GetInstance().EnqueueDownload(
      std::move(request), std::nullopt,
      BindToCurrentSequence(
          BindOnce(&Client::OnResponseStarted, weak_client_, this), FROM_HERE),
      BindToCurrentSequence(
          BindRepeating(&Client::OnWriteData, weak_client_, this), FROM_HERE),
      BindToCurrentSequence(
          BindOnce(&Client::OnRequestFinished, weak_client_, this), FROM_HERE));
}

void UrlRequest::Cancel() {
  if (!cancellation_token_) {
    return;
  }

  NetThread::GetInstance().CancelRequest(std::move(*cancellation_token_));
  weak_factory_.InvalidateWeakPtrs();
}

}  // namespace net
}  // namespace base
