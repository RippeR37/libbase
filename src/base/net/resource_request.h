#pragma once

#if defined(LIBBASE_MODULE_NET)

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "base/time/time_delta.h"

namespace base {
namespace net {

const auto kDefaultHeaders = std::vector<std::string>{};
const auto kNoPostData = std::optional<std::vector<uint8_t>>{};
const auto kNoPost = std::nullopt;
const auto kNoTimeout = base::TimeDelta{};

struct ResourceRequest {
  std::string url;
  std::vector<std::string> headers = kDefaultHeaders;
  std::optional<std::vector<uint8_t>> post_data = kNoPostData;
  bool headers_only = false;

  base::TimeDelta timeout = kNoTimeout;
  base::TimeDelta connect_timeout = kNoTimeout;
  bool follow_redirects = true;

  ResourceRequest WithHeaders(std::vector<std::string> new_headers) const&;
  ResourceRequest&& WithHeaders(std::vector<std::string> new_headers) &&;

  ResourceRequest WithPostData(std::vector<uint8_t> new_post_data) const&;
  ResourceRequest&& WithPostData(std::vector<uint8_t> new_post_data) &&;

  ResourceRequest WithPostData(std::string_view new_post_data) const&;
  ResourceRequest&& WithPostData(std::string_view new_post_data) &&;

  ResourceRequest WithHeadersOnly(bool new_headers_only = true) const&;
  ResourceRequest&& WithHeadersOnly(bool new_headers_only = true) &&;

  ResourceRequest WithTimeout(base::TimeDelta new_timeout) const&;
  ResourceRequest&& WithTimeout(base::TimeDelta new_timeout) &&;

  ResourceRequest WithConnectTimeout(
      base::TimeDelta new_connect_timeout) const&;
  ResourceRequest&& WithConnectTimeout(base::TimeDelta new_connect_timeout) &&;

  ResourceRequest WithFollowRedirects(bool new_follow_redirects = true) const&;
  ResourceRequest&& WithFollowRedirects(bool new_follow_redirects = true) &&;
};

}  // namespace net
}  // namespace base

#endif  // defined(LIBBASE_MODULE_NET)
