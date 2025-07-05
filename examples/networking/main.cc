#include <thread>

#include "base/bind.h"
#include "base/callback.h"
#include "base/init.h"
#include "base/logging.h"
#include "base/message_loop/run_loop.h"
#include "base/net/init.h"
#include "base/net/simple_url_loader.h"
#include "base/net/url_request.h"
#include "base/sequence_checker.h"
#include "base/timer/elapsed_timer.h"

void LogNetResponse(const base::net::ResourceResponse& response) {
  LOG(INFO) << "Result: " << static_cast<int>(response.result);
  LOG(INFO) << "HTTP code: " << response.code;
  LOG(INFO) << "Final URL: " << response.final_url;
  LOG(INFO) << "Downloaded " << response.data.size() << " bytes";
  LOG(INFO) << "Latency: " << response.timing_connect.InMilliseconds() << "ms";
  LOG(INFO) << "Headers";
  for (const auto& [h, v] : response.headers) {
    LOG(INFO) << "  " << h << ": " << v;
  }
  LOG_IF(INFO, !response.data.empty()) << "Content:\n"
                                       << response.DataAsStringView();
}

void NetExampleGet() {
  base::RunLoop run_loop{};

  // Try to download and signal on finish
  base::net::SimpleUrlLoader::DownloadUnbounded(
      base::net::ResourceRequest{"https://www.google.com/robots.txt"}
          .WithHeadersOnly()
          .WithTimeout(base::Seconds(5)),
      base::BindOnce([](base::net::ResourceResponse response) {
        LogNetResponse(response);
      }).Then(run_loop.QuitClosure()));

  // Runs all tasks until the quit callback is called
  run_loop.Run();
}

void NetExamplePost() {
  base::RunLoop run_loop{};

  // Try to download and signal on finish
  base::net::SimpleUrlLoader::DownloadUnbounded(
      base::net::ResourceRequest{"https://httpbin.org/post"}
          .WithHeaders({"Content-Type: application/json"})
          .WithPostData("{\"key\": \"value\"}")
          .WithTimeout(base::Seconds(5)),
      base::BindOnce([](base::net::ResourceResponse response) {
        LogNetResponse(response);
      }).Then(run_loop.QuitClosure()));

  run_loop.Run();
}

class UrlRequestExampleUser : public base::net::UrlRequest::Client {
 public:
  UrlRequestExampleUser(base::OnceClosure finished_closure)
      : finished_closure_(std::move(finished_closure)), request_(this) {}

  void Download(base::net::ResourceRequest request) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    request_.Start(std::move(request));
  }

  void OnResponseStarted(const base::net::UrlRequest* request,
                         int code,
                         std::string final_url,
                         std::map<std::string, std::string> headers) override {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    DCHECK_EQ(request, &request_);

    LOG(INFO) << __FUNCTION__ << "() code: " << code;
    LOG(INFO) << __FUNCTION__ << "() final_url: " << final_url;
    for (const auto& [k, v] : headers) {
      LOG(INFO) << __FUNCTION__ << "() header[" << k << "]: " << v;
    }
  }

  void OnWriteData(const base::net::UrlRequest* request,
                   std::vector<uint8_t> chunk) override {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    DCHECK_EQ(request, &request_);

    LOG(INFO) << __FUNCTION__ << "() received bytes: " << chunk.size();
  }

  void OnRequestFinished(const base::net::UrlRequest* request,
                         base::net::Result result) override {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    DCHECK_EQ(request, &request_);

    LOG(INFO) << __FUNCTION__ << "() result: " << static_cast<int>(result);
    std::move(finished_closure_).Run();
  }

 private:
  SEQUENCE_CHECKER(sequence_checker_);
  base::OnceClosure finished_closure_;
  base::net::UrlRequest request_;
};

void NetExampleUrlRequest() {
  base::RunLoop run_loop;

  UrlRequestExampleUser url_request_user{run_loop.QuitClosure()};
  url_request_user.Download(
      base::net::ResourceRequest{"https://www.google.com/robots.txt"}
          .WithTimeout(base::Seconds(5)));

  run_loop.Run();
}

int main(int argc, char* argv[]) {
  base::Initialize(argc, argv, base::InitOptions{});
  base::net::Initialize(base::net::InitOptions{});

  const auto timer = base::ElapsedTimer{};

  NetExampleGet();
  NetExamplePost();
  NetExampleUrlRequest();

  LOG(INFO) << "Example finished in " << timer.Elapsed().InMillisecondsF()
            << "ms";

  base::net::Deinitialize();
  base::Deinitialize();
  return 0;
}
