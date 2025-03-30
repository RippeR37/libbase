#pragma once

#include <cstddef>

namespace base {
namespace net {

struct RequestCancellationToken {
  explicit RequestCancellationToken(size_t request_id);

  bool operator==(const RequestCancellationToken& other) const;

 private:
  size_t request_id_;
};

}  // namespace net
}  // namespace base
