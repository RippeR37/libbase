#pragma once

#include "base/callback.h"
#include "base/net/request_cancellation_token.h"
#include "base/net/resource_request.h"
#include "base/net/resource_response.h"

namespace base {
namespace net {

class SimpleUrlLoader {
 public:
  using ResultCallback = base::OnceCallback<void(ResourceResponse)>;

  static RequestCancellationToken DownloadUnbounded(
      ResourceRequest request,
      ResultCallback on_done_callback);
  static RequestCancellationToken DownloadLimited(
      ResourceRequest request,
      size_t max_response_size_bytes,
      ResultCallback on_done_callback);

  static void CancelRequest(RequestCancellationToken cancellation_token);

 private:
  //
};

}  // namespace net
}  // namespace base
