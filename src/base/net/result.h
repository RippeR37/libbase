#pragma once

#if defined(LIBBASE_MODULE_NET)

namespace base {
namespace net {

enum class Result {
  //
  kOk,

  //
  kError,

  //
  kTimeout,

  //
  kAborted,
};

}  // namespace net
}  // namespace base

#endif  // defined(LIBBASE_MODULE_NET)
