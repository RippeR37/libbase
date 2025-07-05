#include "base/net/resource_response.h"

namespace base {
namespace net {

std::string ResourceResponse::DataAsString() const {
  return std::string{data.begin(), data.end()};
}

std::string_view ResourceResponse::DataAsStringView() const {
  return std::string_view{reinterpret_cast<const char*>(data.data()), data.size()};
}

}  // namespace net
}  // namespace base
