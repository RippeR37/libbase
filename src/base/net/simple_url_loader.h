#pragma once

#include "base/callback.h"
#include "base/net/resource_request.h"
#include "base/net/resource_response.h"

namespace base {
namespace net {

class SimpleUrlLoader {
 public:
  using ResultCallback = base::OnceCallback<void(ResourceResponse)>;

  static void DownloadUnbounded(ResourceRequest request,
                                ResultCallback on_done_callback);
  static void DownloadLimited(ResourceRequest request,
                              size_t max_response_size_bytes,
                              ResultCallback on_done_callback);

 private:
  //
};

}  // namespace net
}  // namespace base
