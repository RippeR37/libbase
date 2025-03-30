#include "base/net/impl/net_thread_impl.h"

namespace base {
namespace net {

namespace {
Result CurlCodeToNetResult(CURLcode code) {
  switch (code) {
    case CURLE_OK:
      return Result::kOk;

    case CURLE_ABORTED_BY_CALLBACK:
      return Result::kAborted;

    default:
      return Result::kError;
  }
}
}  // namespace

struct NetThread::NetThreadImpl::DownloadInfo {
  ResourceRequest request;
  std::optional<size_t> max_response_size_bytes;

  struct curl_slist* headers = nullptr;

  ResourceResponse response;
  OnceCallback<void(ResourceResponse)> on_done_callback;
};

//
// Logic executed on main thread
//

NetThread::NetThreadImpl::NetThreadImpl()
    : not_quit_(), not_modified_(), multi_handle_(nullptr) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

NetThread::NetThreadImpl::~NetThreadImpl() {
  if (thread_) {
    Stop();
  }

  curl_global_cleanup();
}

void NetThread::NetThreadImpl::Start() {
  CHECK(!thread_);

  not_quit_.test_and_set();
  not_modified_.test_and_set();

  multi_handle_ = curl_multi_init();
  thread_ =
      std::make_unique<std::thread>(&NetThreadImpl::RunLoop_NetThread, this);
}

void NetThread::NetThreadImpl::Stop() {
  not_quit_.clear();
  curl_multi_wakeup(multi_handle_);

  thread_->join();
  thread_.reset();

  curl_multi_cleanup(multi_handle_);
  multi_handle_ = nullptr;
}

void NetThread::NetThreadImpl::EnqueueDownload(
    ResourceRequest request,
    std::optional<size_t> max_response_size_bytes,
    OnceCallback<void(ResourceResponse)> on_done_callback) {
  DCHECK(thread_);
  DCHECK(on_done_callback);

  {
    std::unique_lock<std::mutex> lock(mutex_);
    pending_add_downloads_.push_back(DownloadInfo{
        std::move(request),
        std::move(max_response_size_bytes),
        nullptr,
        {},
        std::move(on_done_callback),
    });
  }

  not_modified_.clear();
  curl_multi_wakeup(multi_handle_);
}

//
// Logic executed on NetThread
//

void NetThread::NetThreadImpl::RunLoop_NetThread() {
  while (not_quit_.test_and_set()) {
    ProcessPendingActions_NetThread();
    Perform_NetThread();
    ProcessCompleted_NetThread();
    Wait_NetThread();
  }

  AbortAllDownloads_NetThread();
}

void NetThread::NetThreadImpl::ProcessPendingActions_NetThread() {
  if (!not_modified_.test_and_set()) {
    std::unique_lock<std::mutex> lock(mutex_);
    for (DownloadInfo& download : pending_add_downloads_) {
      EnqueueDownload_NetThread(download);
    }
    pending_add_downloads_.clear();
    // TODO: handle pending_cancel_downloads_
  }
}

void NetThread::NetThreadImpl::Perform_NetThread() {
  int active = 0;
  CURLMcode result = CURLM_OK;
  do {
    result = curl_multi_perform(multi_handle_, &active);
  } while (result == CURLM_CALL_MULTI_PERFORM);

  if (result != CURLM_OK) {
    LOG(ERROR) << __FUNCTION__ << "() curl_multi_perform failed: "
               << curl_multi_strerror(result);
    AbortAllDownloads_NetThread();
  }
}

void NetThread::NetThreadImpl::ProcessCompleted_NetThread() {
  CURLMsg* msg = nullptr;
  int msgs_left = 0;
  while ((msg = curl_multi_info_read(multi_handle_, &msgs_left))) {
    if (msg->msg == CURLMSG_DONE) {
      DownloadFinished_NetThread(msg->easy_handle,
                                 CurlCodeToNetResult(msg->data.result));
    }
  }
}

void NetThread::NetThreadImpl::Wait_NetThread() {
  const auto kWaitTimeout = base::Milliseconds(1000);

  int numfds = 0;
  CURLMcode result =
      curl_multi_poll(multi_handle_, nullptr, 0,
                      static_cast<int>(kWaitTimeout.InMilliseconds()), &numfds);
  if (result != CURLM_OK) {
    LOG(ERROR) << __FUNCTION__
               << "() curl_multi_poll failed: " << curl_multi_strerror(result);
  }
}

void NetThread::NetThreadImpl::EnqueueDownload_NetThread(
    DownloadInfo& download_info) {
  CURL* easy_handle = curl_easy_init();

  // Set easy handle's options
  curl_easy_setopt(easy_handle, CURLOPT_PRIVATE, this);
  curl_easy_setopt(easy_handle, CURLOPT_URL, download_info.request.url.c_str());
  if (download_info.request.follow_redirects) {
    curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
  }
  if (download_info.request.headers_only) {
    curl_easy_setopt(easy_handle, CURLOPT_NOBODY, 1L);
  }
  for (const auto& header : download_info.request.headers) {
    download_info.headers =
        curl_slist_append(download_info.headers, header.c_str());
  }
  if (download_info.headers) {
    curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, download_info.headers);
  }
  if (!download_info.request.connect_timeout.IsZero()) {
    curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT_MS,
                     download_info.request.connect_timeout.InMilliseconds());
  }
  if (!download_info.request.timeout.IsZero()) {
    curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT_MS,
                     download_info.request.timeout.InMilliseconds());
  }

  // Save download request/response data
  active_downloads_.insert({easy_handle, std::move(download_info)});
  const auto& inserted_info = active_downloads_[easy_handle];

  // WARNING: Lambda has to be converted to function pointer or it will crash!
  curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, (void*)(&inserted_info));
  curl_easy_setopt(
      easy_handle, CURLOPT_WRITEFUNCTION,
      +[](char* data, size_t n, size_t l, void* userp) -> size_t {
        auto* info = static_cast<DownloadInfo*>(userp);
        if (info->max_response_size_bytes &&
            info->response.data.size() + (n * l) >
                *info->max_response_size_bytes) {
          // Force error
          return 0;
        }
        info->response.data.insert(info->response.data.end(),
                                   reinterpret_cast<uint8_t*>(data),
                                   reinterpret_cast<uint8_t*>(data) + n * l);
        return n * l;
      });

  curl_multi_add_handle(multi_handle_, easy_handle);
}

void NetThread::NetThreadImpl::DownloadFinished_NetThread(CURL* finished_curl,
                                                          Result result) {
  CHECK_GT(active_downloads_.count(finished_curl), 0);

  auto& download = active_downloads_[finished_curl];

  // Result
  download.response.result = result;

  // Response code
  if (download.response.result == Result::kOk) {
    long http_code = 0;
    curl_easy_getinfo(finished_curl, CURLINFO_RESPONSE_CODE, &http_code);
    download.response.code = static_cast<int>(http_code);
  } else {
    download.response.code = -1;
  }

  // Get the URL of the completed transfer
  {
    char* final_url = nullptr;
    curl_easy_getinfo(finished_curl, CURLINFO_EFFECTIVE_URL, &final_url);

    download.response.final_url = final_url ? final_url : "";
  }

  // Get final headers of the completed transfer
  {
    struct curl_header* header = nullptr;
    while ((header =
                curl_easy_nextheader(finished_curl, CURLH_HEADER, 0, header))) {
      download.response.headers.insert({header->name, header->value});
    }
  }

  // Get timing data
  {
    curl_off_t time = {};
    if (curl_easy_getinfo(finished_curl, CURLINFO_QUEUE_TIME_T, &time) ==
        CURLE_OK) {
      download.response.timing_queue = Microseconds(time);
    }
    if (curl_easy_getinfo(finished_curl, CURLINFO_CONNECT_TIME_T, &time) ==
        CURLE_OK) {
      download.response.timing_connect = Microseconds(time);
    }
    if (curl_easy_getinfo(finished_curl, CURLINFO_STARTTRANSFER_TIME_T,
                          &time) == CURLE_OK) {
      download.response.timing_start_transfer = Microseconds(time);
    }
    if (curl_easy_getinfo(finished_curl, CURLINFO_TOTAL_TIME_T, &time) ==
        CURLE_OK) {
      download.response.timing_total = Microseconds(time);
    }
  }

  DCHECK(download.on_done_callback);
  std::move(download.on_done_callback).Run(std::move(download.response));

  RemoveDownload_NetThread(finished_curl);
}

void NetThread::NetThreadImpl::RemoveDownload_NetThread(CURL* finished_curl) {
  // Remove the completed request from multi-handle
  curl_multi_remove_handle(multi_handle_, finished_curl);
  if (active_downloads_.count(finished_curl) > 0) {
    if (active_downloads_[finished_curl].headers) {
      curl_slist_free_all(active_downloads_[finished_curl].headers);
    }
  }
  curl_easy_cleanup(finished_curl);

  // Remove the handle from our list
  active_downloads_.erase(finished_curl);
}

void NetThread::NetThreadImpl::AbortAllDownloads_NetThread() {
  for (auto& [easy_handle, info] : active_downloads_) {
    std::move(info.on_done_callback)
        .Run(ResourceResponse{Result::kAborted, -1, {}, {}, {}, {}, {}, {}});

    curl_multi_remove_handle(multi_handle_, easy_handle);
    if (info.headers) {
      curl_slist_free_all(info.headers);
    }
    curl_easy_cleanup(easy_handle);
  }

  active_downloads_.clear();
}

}  // namespace net
}  // namespace base
