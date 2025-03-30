#pragma once

#include <memory>
#include <optional>

#include "base/callback.h"
#include "base/net/request_cancellation_token.h"
#include "base/net/resource_request.h"
#include "base/net/resource_response.h"

namespace base {
namespace net {

class NetThread {
 public:
  static NetThread& GetInstance();

  void Start();
  void Stop();

  RequestCancellationToken EnqueueDownload(
      ResourceRequest request,
      std::optional<size_t> max_response_size_bytes,
      OnceCallback<void(ResourceResponse)> on_done_callback);

  void CancelRequest(RequestCancellationToken cancellation_token);

 private:
  class NetThreadImpl;

  std::unique_ptr<NetThreadImpl> impl_;
};

}  // namespace net
}  // namespace base
