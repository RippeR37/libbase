#include "base/net/simple_url_loader.h"

#include <optional>

#include "base/bind_post_task.h"
#include "base/net/impl/net_thread.h"

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
}  // namespace

RequestCancellationToken SimpleUrlLoader::DownloadUnbounded(
    ResourceRequest request,
    ResultCallback on_done_callback,
    std::shared_ptr<TaskRunner> reply_task_runner) {
  return NetThread::GetInstance().EnqueueDownload(
      request, std::nullopt,
      BindOnDoneCallback(std::move(on_done_callback),
                         std::move(reply_task_runner)));
}

RequestCancellationToken SimpleUrlLoader::DownloadLimited(
    ResourceRequest request,
    size_t max_response_size_bytes,
    ResultCallback on_done_callback,
    std::shared_ptr<TaskRunner> reply_task_runner) {
  return NetThread::GetInstance().EnqueueDownload(
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
