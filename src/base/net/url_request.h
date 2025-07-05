#pragma once

#if defined(LIBBASE_MODULE_NET)

#include <map>
#include <optional>
#include <string>

#include "base/memory/weak_ptr.h"
#include "base/net/request_cancellation_token.h"
#include "base/net/resource_request.h"
#include "base/net/result.h"

namespace base {
namespace net {

class UrlRequest {
 public:
  class Client {
   public:
    virtual ~Client() = default;
    virtual void OnResponseStarted(
        const UrlRequest* request,
        int code,
        std::string final_url,
        std::map<std::string, std::string> headers) = 0;
    virtual void OnWriteData(const UrlRequest* request,
                             std::vector<uint8_t> chunk) = 0;
    virtual void OnRequestFinished(const UrlRequest* request,
                                   Result result) = 0;
  };

  UrlRequest(Client* client);
  ~UrlRequest();

  UrlRequest(const UrlRequest&) = delete;
  UrlRequest(UrlRequest&&) = delete;

  UrlRequest& operator=(const UrlRequest&) = delete;
  UrlRequest& operator=(UrlRequest&&) = delete;

  void Start(ResourceRequest request);
  void Cancel();

 private:
  std::optional<RequestCancellationToken> cancellation_token_;

  base::WeakPtr<Client> weak_client_;
  base::WeakPtrFactory<Client> weak_factory_;
};

}  // namespace net
}  // namespace base

#endif  // defined(LIBBASE_MODULE_NET)
