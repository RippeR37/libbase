#pragma once

#if defined(LIBBASE_MODULE_NET)

#include <map>
#include <memory>
#include <optional>
#include <string>

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
  RequestCancellationToken EnqueueDownload(
      ResourceRequest request,
      std::optional<size_t> max_response_size_bytes,
      OnceCallback<void(int, std::string, std::map<std::string, std::string>)>
          on_response_started,
      RepeatingCallback<void(std::vector<uint8_t>)> on_write_data,
      OnceCallback<void(Result)> on_finished);

  void CancelRequest(RequestCancellationToken cancellation_token);

 private:
  class NetThreadImpl;

  std::unique_ptr<NetThreadImpl> impl_;
};

}  // namespace net
}  // namespace base

#endif  // defined(LIBBASE_MODULE_NET)
