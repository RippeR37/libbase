#include "base/net/request_cancellation_token.h"

namespace base {
namespace net {

RequestCancellationToken::RequestCancellationToken(size_t request_id)
    : request_id_(request_id) {}

bool RequestCancellationToken::operator==(
    const RequestCancellationToken& other) const {
  return request_id_ == other.request_id_;
}

}  // namespace net
}  // namespace base
