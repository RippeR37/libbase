#include "base/net/resource_request.h"

namespace base {
namespace net {

ResourceRequest ResourceRequest::WithHeaders(
    std::vector<std::string> new_headers) const& {
  return ResourceRequest{*this}.WithHeaders(std::move(new_headers));
}

ResourceRequest&& ResourceRequest::WithHeaders(
    std::vector<std::string> new_headers) && {
  headers = std::move(new_headers);
  return std::move(*this);
}

ResourceRequest ResourceRequest::WithPostData(
    std::vector<uint8_t> new_post_data) const& {
  return ResourceRequest{*this}.WithPostData(std::move(new_post_data));
}

ResourceRequest&& ResourceRequest::WithPostData(
    std::vector<uint8_t> new_post_data) && {
  post_data = std::move(new_post_data);
  return std::move(*this);
}

ResourceRequest ResourceRequest::WithPostData(
    std::string_view new_post_data) const& {
  return ResourceRequest{*this}.WithPostData(std::move(new_post_data));
}

ResourceRequest&& ResourceRequest::WithPostData(
    std::string_view new_post_data) && {
  post_data = std::vector<uint8_t>(new_post_data.begin(), new_post_data.end());
  return std::move(*this);
}

ResourceRequest ResourceRequest::WithHeadersOnly(bool new_headers_only) const& {
  return ResourceRequest{*this}.WithHeadersOnly(std::move(new_headers_only));
}

ResourceRequest&& ResourceRequest::WithHeadersOnly(bool new_headers_only) && {
  headers_only = std::move(new_headers_only);
  return std::move(*this);
}

ResourceRequest ResourceRequest::WithTimeout(
    base::TimeDelta new_timeout) const& {
  return ResourceRequest{*this}.WithTimeout(std::move(new_timeout));
}

ResourceRequest&& ResourceRequest::WithTimeout(base::TimeDelta new_timeout) && {
  timeout = std::move(new_timeout);
  return std::move(*this);
}

ResourceRequest ResourceRequest::WithConnectTimeout(
    base::TimeDelta new_connect_timeout) const& {
  return ResourceRequest{*this}.WithConnectTimeout(
      std::move(new_connect_timeout));
}

ResourceRequest&& ResourceRequest::WithConnectTimeout(
    base::TimeDelta new_connect_timeout) && {
  connect_timeout = std::move(new_connect_timeout);
  return std::move(*this);
}

ResourceRequest ResourceRequest::WithFollowRedirects(
    bool new_follow_redirects) const& {
  return ResourceRequest{*this}.WithFollowRedirects(
      std::move(new_follow_redirects));
}

ResourceRequest&& ResourceRequest::WithFollowRedirects(
    bool new_follow_redirects) && {
  follow_redirects = std::move(new_follow_redirects);
  return std::move(*this);
}

}  // namespace net
}  // namespace base
