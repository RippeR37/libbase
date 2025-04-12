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
                       RequestCancellationToken cancellation_token,
                       OnceCallback<void(ResourceResponse)> on_done_callback);
  void EnqueueDownload(
      ResourceRequest request,
      std::optional<size_t> max_response_size_bytes,
      RequestCancellationToken cancellation_token,
      OnceCallback<void(int, std::string, std::map<std::string, std::string>)>
          on_response_started,
      RepeatingCallback<void(std::vector<uint8_t>)> on_write_data,
      OnceCallback<void(Result)> on_finished);

  void CancelRequest(RequestCancellationToken cancellation_token);

 private:
  struct DownloadInfo;

  void RunLoop_NetThread();

  // Parts of RunLoop
  void ProcessPendingActions_NetThread();
  void Perform_NetThread();
  void ProcessCompleted_NetThread();
  void Wait_NetThread();

  void EnqueueDownload_NetThread(DownloadInfo& download_info);
  void CancelRequest_NetThread(RequestCancellationToken cancellation_token);
  void DownloadFinished_NetThread(CURL* finished_curl, Result result);
  void RemoveDownload_NetThread(CURL* finished_curl);
  void AbortAllDownloads_NetThread();

  CURL* FindHandleByCancellationToken_NetThread(
      RequestCancellationToken cancellation_token) const;

  // Accessible anywhere
  std::atomic_flag not_quit_;
  std::atomic_flag not_modified_;

  // Accessible with mutex
  std::mutex mutex_;
  std::vector<DownloadInfo> pending_add_downloads_;
  std::vector<RequestCancellationToken> pending_cancel_downloads_;

  // Accessible on `thread_`
  CURLM* multi_handle_;
  std::map<CURL*, DownloadInfo> active_downloads_;

  // Accessible on IoThread owner thread
  std::unique_ptr<std::thread> thread_;
};

}  // namespace net
}  // namespace base
