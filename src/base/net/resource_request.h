#pragma once

#include <string>
#include <vector>

#include "base/time/time_delta.h"

namespace base {
namespace net {

const auto kDefaultHeaders = std::vector<std::string>{};
const auto kNoTimeout = base::TimeDelta{};

struct ResourceRequest {
  std::string url;
  std::vector<std::string> headers = kDefaultHeaders;

  base::TimeDelta connect_timeout = kNoTimeout;
  base::TimeDelta timeout = kNoTimeout;
  bool follow_redirects = true;
  bool headers_only = false;
};

}  // namespace net
}  // namespace base
