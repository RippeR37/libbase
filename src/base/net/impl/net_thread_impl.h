#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <vector>

#include "base/callback.h"
#include "base/net/impl/net_thread.h"
#include "base/net/resource_request.h"
#include "base/net/resource_response.h"
#include "base/net/result.h"

#include "curl/curl.h"

namespace base {
namespace net {

class NetThread::NetThreadImpl {
 public:
  NetThreadImpl();
  ~NetThreadImpl();

  void Start();
  void Stop();

  void EnqueueDownload(ResourceRequest request,
                       std::optional<size_t> max_response_size_bytes,
                       OnceCallback<void(ResourceResponse)> on_done_callback);

 private:
  struct DownloadInfo;

  void RunLoop_NetThread();

  // Parts of RunLoop
  void ProcessPendingActions_NetThread();
  void Perform_NetThread();
  void ProcessCompleted_NetThread();
  void Wait_NetThread();

  void EnqueueDownload_NetThread(DownloadInfo& download_info);
  void DownloadFinished_NetThread(CURL* finished_curl, Result result);
  void RemoveDownload_NetThread(CURL* finished_curl);
  void AbortAllDownloads_NetThread();

  // Accessible anywhere
  std::atomic_flag not_quit_;
  std::atomic_flag not_modified_;

  // Accessible with mutex
  std::mutex mutex_;
  std::vector<DownloadInfo> pending_add_downloads_;
  // TODO: add pending_cancel_downloads_;

  // Accessible on `thread_`
  CURLM* multi_handle_;
  std::map<CURL*, DownloadInfo> active_downloads_;

  // Accessible on IoThread owner thread
  std::unique_ptr<std::thread> thread_;
};

}  // namespace net
}  // namespace base
