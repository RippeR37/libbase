#include "base/net/simple_url_loader.h"

#include <optional>

#include "base/bind_post_task.h"
#include "base/net/impl/net_thread.h"
#include "base/timer/elapsed_timer.h"

namespace base {
namespace net {

namespace {
SimpleUrlLoader::ResultCallback BindOnDoneCallback(
    SimpleUrlLoader::ResultCallback callback,
    std::shared_ptr<TaskRunner> reply_task_runner) {
  // TODO: Fix FROM_HERE usage here
  if (reply_task_runner) {
    return BindPostTask(std::move(reply_task_runner), std::move(callback),
                        FROM_HERE);
  }
  return BindToCurrentSequence(std::move(callback), FROM_HERE);
}

class SimpleUrlRequestWrapper {
 public:
  static RequestCancellationToken Download(
      base::net::ResourceRequest request,
      std::optional<size_t> max_response_size_bytes,
      SimpleUrlLoader::ResultCallback on_done_callback) {
    std::unique_ptr<SimpleUrlRequestWrapper> wrapper{
        new SimpleUrlRequestWrapper(std::move(on_done_callback))};

    // Need to create these one-by-one earlier to ensure order
    auto on_response_started_cb =
        base::BindOnce(&SimpleUrlRequestWrapper::OnResponseStarted,
                       base::Unretained(wrapper.get()));
    auto on_write_data_cb = base::BindRepeating(
        &SimpleUrlRequestWrapper::OnWriteData, base::Unretained(wrapper.get()));
    auto on_request_finished_cb =
        base::BindOnce(&SimpleUrlRequestWrapper::OnRequestFinished,
                       base::Owned(std::move(wrapper)));

    return NetThread::GetInstance().EnqueueDownload(
        std::move(request), std::move(max_response_size_bytes),
        std::move(on_response_started_cb), std::move(on_write_data_cb),
        std::move(on_request_finished_cb));
  }

 private:
  SimpleUrlRequestWrapper(SimpleUrlLoader::ResultCallback on_done_callback)
      : on_done_callback_(std::move(on_done_callback)) {
    DETACH_FROM_SEQUENCE(sequence_checker_);
  }

  void OnResponseStarted(int code,
                         std::string final_url,
                         std::map<std::string, std::string> headers) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

    response_.code = code;
    response_.final_url = std::move(final_url);
    response_.headers = std::move(headers);

    // If ContentLength is known, try to reserve buffer for it upfront
    const auto* kContentLengthHeader = "Content-Length";
    if (response_.headers.count(kContentLengthHeader)) {
      const size_t content_length =
          std::stoull(response_.headers[kContentLengthHeader]);

      const auto kMaxResponseReserve = 10 * 1024 * 1024;  // 10MB
      if (content_length < kMaxResponseReserve) {
        response_.data.reserve(content_length);
      }
    }

    response_.timing_queue = base::Milliseconds(0);
    response_.timing_connect = since_start_timer_.Elapsed();
  }

  void OnWriteData(std::vector<uint8_t> chunk) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    if (response_.data.empty()) {
      response_.timing_start_transfer = since_start_timer_.Elapsed();
    }
    response_.data.reserve(response_.data.size() + chunk.size());
    response_.data.insert(response_.data.end(), chunk.begin(), chunk.end());
  }

  void OnRequestFinished(base::net::Result result) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

    response_.timing_total = since_start_timer_.Elapsed();
    response_.result = result;
    std::move(on_done_callback_).Run(std::move(response_));
  }

  SEQUENCE_CHECKER(sequence_checker_);
  base::ElapsedTimer since_start_timer_;
  SimpleUrlLoader::ResultCallback on_done_callback_;
  ResourceResponse response_;
};
}  // namespace

RequestCancellationToken SimpleUrlLoader::DownloadUnbounded(
    ResourceRequest request,
    ResultCallback on_done_callback,
    std::shared_ptr<TaskRunner> reply_task_runner) {
  return SimpleUrlRequestWrapper::Download(
      request, std::nullopt,
      BindOnDoneCallback(std::move(on_done_callback),
                         std::move(reply_task_runner)));
}

RequestCancellationToken SimpleUrlLoader::DownloadLimited(
    ResourceRequest request,
    size_t max_response_size_bytes,
    ResultCallback on_done_callback,
    std::shared_ptr<TaskRunner> reply_task_runner) {
  return SimpleUrlRequestWrapper::Download(
      request, max_response_size_bytes,
      BindOnDoneCallback(std::move(on_done_callback),
                         std::move(reply_task_runner)));
}

void SimpleUrlLoader::CancelRequest(
    RequestCancellationToken cancellation_token) {
  return NetThread::GetInstance().CancelRequest(std::move(cancellation_token));
}

}  // namespace net
}  // namespace base
