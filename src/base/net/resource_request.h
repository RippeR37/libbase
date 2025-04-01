#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "base/time/time_delta.h"

namespace base {
namespace net {

const auto kDefaultHeaders = std::vector<std::string>{};
const auto kNoPost = std::nullopt;
const auto kNoTimeout = base::TimeDelta{};

struct ResourceRequest {
  std::string url;
  std::vector<std::string> headers = kDefaultHeaders;
  std::optional<std::vector<uint8_t>> post_data;
  bool headers_only = false;

  base::TimeDelta connect_timeout = kNoTimeout;
  base::TimeDelta timeout = kNoTimeout;
  bool follow_redirects = true;
};

}  // namespace net
}  // namespace base
