#include "base/net/impl/net_thread_impl.h"

namespace base {
namespace net {

namespace {
Result CurlCodeToNetResult(CURLcode code) {
  switch (code) {
    case CURLE_OK:
      return Result::kOk;

    case CURLE_OPERATION_TIMEDOUT:
      return Result::kTimeout;

    case CURLE_ABORTED_BY_CALLBACK:
      return Result::kAborted;

    default:
      return Result::kError;
  }
}

std::tuple<int, std::string, std::map<std::string, std::string>>
GetResponseInfo(CURL* handle, Result result = Result::kOk) {
  int code = -1;
  std::string final_url;
  std::map<std::string, std::string> headers;

  // Response code
  if (result == Result::kOk || result == Result::kAborted) {
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
  }

  // Get the URL of the completed transfer
  {
    char* final_url_cstr = nullptr;
    curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &final_url_cstr);
    final_url = final_url_cstr ? final_url_cstr : "";
  }

  // Get final headers of the completed transfer
  {
    struct curl_header* header = nullptr;
    while ((header = curl_easy_nextheader(handle, CURLH_HEADER, 0, header))) {
      headers.insert({header->name, header->value});
    }
  }

  return {code, final_url, headers};
}
}  // namespace

struct NetThread::NetThreadImpl::DownloadInfo {
  CURL* handle;
  ResourceRequest request;
  std::optional<size_t> max_response_size_bytes;
  RequestCancellationToken cancellation_token;

  struct curl_slist* headers = nullptr;

  // Simple download
  ResourceResponse response;
  OnceCallback<void(ResourceResponse)> on_done_callback;

  // Advanced download
  OnceCallback<void(int, std::string, std::map<std::string, std::string>)>
      on_response_started;
  RepeatingCallback<void(std::vector<uint8_t>)> on_write_data;
  OnceCallback<void(Result)> on_finished;
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
    RequestCancellationToken cancellation_token,
    OnceCallback<void(ResourceResponse)> on_done_callback) {
  DCHECK(thread_);
  DCHECK(on_done_callback);

  {
    std::unique_lock<std::mutex> lock(mutex_);
    pending_add_downloads_.push_back(DownloadInfo{
        nullptr,
        std::move(request),
        std::move(max_response_size_bytes),
        std::move(cancellation_token),
        nullptr,
        {},
        std::move(on_done_callback),
        {},
        {},
        {},
    });
  }

  not_modified_.clear();
  curl_multi_wakeup(multi_handle_);
}

void NetThread::NetThreadImpl::EnqueueDownload(
    ResourceRequest request,
    std::optional<size_t> max_response_size_bytes,
    RequestCancellationToken cancellation_token,
    OnceCallback<void(int, std::string, std::map<std::string, std::string>)>
        on_response_started,
    RepeatingCallback<void(std::vector<uint8_t>)> on_write_data,
    OnceCallback<void(Result)> on_finished) {
  DCHECK(thread_);
  DCHECK(on_response_started);
  DCHECK(on_write_data);
  DCHECK(on_finished);

  {
    std::unique_lock<std::mutex> lock(mutex_);
    pending_add_downloads_.push_back(DownloadInfo{
        nullptr,
        std::move(request),
        std::move(max_response_size_bytes),
        std::move(cancellation_token),
        nullptr,
        {},
        {},
        std::move(on_response_started),
        std::move(on_write_data),
        std::move(on_finished),
    });
  }

  not_modified_.clear();
  curl_multi_wakeup(multi_handle_);
}

void NetThread::NetThreadImpl::CancelRequest(
    RequestCancellationToken cancellation_token) {
  DCHECK(thread_);

  {
    std::unique_lock<std::mutex> lock(mutex_);
    pending_cancel_downloads_.push_back(std::move(cancellation_token));
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

    for (RequestCancellationToken cancellation_token :
         pending_cancel_downloads_) {
      if (CURL* cancelled_handle =
              FindHandleByCancellationToken_NetThread(cancellation_token)) {
        DownloadFinished_NetThread(cancelled_handle, Result::kAborted);
      }
    }
    pending_cancel_downloads_.clear();
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

  download_info.handle = easy_handle;

  // Set easy handle's options
  curl_easy_setopt(easy_handle, CURLOPT_PRIVATE, this);
  curl_easy_setopt(easy_handle, CURLOPT_URL, download_info.request.url.c_str());
  if (download_info.request.follow_redirects) {
    curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
  }

  // POST or HEAD or GET
  if (download_info.request.post_data) {
    curl_easy_setopt(easy_handle, CURLOPT_POST, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS,
                     download_info.request.post_data->data());
    curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDSIZE_LARGE,
                     download_info.request.post_data->size());
  }
  if (download_info.request.headers_only) {
    curl_easy_setopt(easy_handle, CURLOPT_NOBODY, 1L);
  }

  // Headers
  for (const auto& header : download_info.request.headers) {
    download_info.headers =
        curl_slist_append(download_info.headers, header.c_str());
  }
  if (download_info.headers) {
    curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, download_info.headers);
  }

  // Timeouts
  if (!download_info.request.connect_timeout.IsZero()) {
    curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT_MS,
                     download_info.request.connect_timeout.InMilliseconds());
  }
  if (!download_info.request.timeout.IsZero()) {
    curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT_MS,
                     download_info.request.timeout.InMilliseconds());
  }

  // Save download request/response data
  auto [iter, inserted] =
      active_downloads_.emplace(easy_handle, std::move(download_info));
  DCHECK(inserted);
  const auto& inserted_info = iter->second;

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

        if (info->on_response_started) {
          auto response_info = GetResponseInfo(info->handle);
          std::move(info->on_response_started)
              .Run(std::get<0>(response_info),
                   std::move(std::get<1>(response_info)),
                   std::move(std::get<2>(response_info)));
        }

        if (info->on_write_data) {
          // Send chunk right away
          std::vector<uint8_t> chunk(
              reinterpret_cast<uint8_t*>(data),
              reinterpret_cast<uint8_t*>(data) + (n * l));
          info->on_write_data.Run(std::move(chunk));
        } else {
          // Store data which will be sent on finish/error
          info->response.data.insert(
              info->response.data.end(), reinterpret_cast<uint8_t*>(data),
              reinterpret_cast<uint8_t*>(data) + (n * l));
        }
        return n * l;
      });

  curl_multi_add_handle(multi_handle_, easy_handle);
}

void NetThread::NetThreadImpl::DownloadFinished_NetThread(CURL* finished_curl,
                                                          Result result) {
  auto download_iter = active_downloads_.find(finished_curl);
  CHECK(download_iter != active_downloads_.end());

  auto& download = download_iter->second;

  // Handle advanced path and just send status & do cleanup
  if (download.on_response_started) {
    auto response_info = GetResponseInfo(finished_curl);
    std::move(download.on_response_started)
        .Run(std::get<0>(response_info), std::move(std::get<1>(response_info)),
             std::move(std::get<2>(response_info)));
  }
  if (download.on_finished) {
    std::move(download.on_finished).Run(result);
    RemoveDownload_NetThread(finished_curl);
    return;
  }

  LOG(ERROR) << __FUNCTION__ << "(###) on_done path";

  // Handle simple path and send everything

  // Result
  download.response.result = result;

  // Response info
  std::tie(download.response.code, download.response.final_url,
           download.response.headers) = GetResponseInfo(finished_curl, result);

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

  // Save on-done callback for later, but first remove and clean up all CURL
  // resources to ensure it doesn't use anything provided anymore.
  DCHECK(download.on_done_callback);
  auto on_done_callback = std::move(download.on_done_callback);
  auto response = std::move(download.response);

  RemoveDownload_NetThread(finished_curl);

  std::move(on_done_callback).Run(std::move(response));
}

void NetThread::NetThreadImpl::RemoveDownload_NetThread(CURL* finished_curl) {
  // Remove the completed request from multi-handle
  curl_multi_remove_handle(multi_handle_, finished_curl);

  auto download_iter = active_downloads_.find(finished_curl);
  if (download_iter != active_downloads_.end()) {
    auto& download = download_iter->second;
    if (download.headers) {
      curl_slist_free_all(download.headers);
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

CURL* NetThread::NetThreadImpl::FindHandleByCancellationToken_NetThread(
    RequestCancellationToken cancellation_token) const {
  for (const auto& [easy_handle, info] : active_downloads_) {
    if (info.cancellation_token == cancellation_token) {
      return easy_handle;
    }
  }
  return nullptr;
}

}  // namespace net
}  // namespace base
