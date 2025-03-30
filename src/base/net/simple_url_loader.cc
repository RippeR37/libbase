#include "base/net/simple_url_loader.h"

#include <optional>

#include "base/net/impl/net_thread.h"

namespace base {
namespace net {

void SimpleUrlLoader::DownloadUnbounded(ResourceRequest request,
                                        ResultCallback on_done_callback) {
  NetThread::GetInstance().EnqueueDownload(request, std::nullopt,
                                           std::move(on_done_callback));
}

void SimpleUrlLoader::DownloadLimited(ResourceRequest request,
                                      size_t max_response_size_bytes,
                                      ResultCallback on_done_callback) {
  NetThread::GetInstance().EnqueueDownload(request, max_response_size_bytes,
                                           std::move(on_done_callback));
}

}  // namespace net
}  // namespace base
