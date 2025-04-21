#pragma once

#if defined(LIBBASE_MODULE_NET)

#include "base/callback.h"
#include "base/net/request_cancellation_token.h"
#include "base/net/resource_request.h"
#include "base/net/resource_response.h"
#include "base/task_runner.h"

namespace base {
namespace net {

class SimpleUrlLoader {
 public:
  using ResultCallback = OnceCallback<void(ResourceResponse)>;

  static RequestCancellationToken DownloadUnbounded(
      ResourceRequest request,
      ResultCallback on_done_callback,
      std::shared_ptr<TaskRunner> reply_task_runner = nullptr);
  static RequestCancellationToken DownloadLimited(
      ResourceRequest request,
      size_t max_response_size_bytes,
      ResultCallback on_done_callback,
      std::shared_ptr<TaskRunner> reply_task_runner = nullptr);

  static void CancelRequest(RequestCancellationToken cancellation_token);
};

}  // namespace net
}  // namespace base

#endif  // defined(LIBBASE_MODULE_NET)
