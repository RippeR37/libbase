#pragma once

#if defined(LIBBASE_MODULE_NET)

namespace base {
namespace net {

struct InitOptions {};

void Initialize(InitOptions options);
void Deinitialize();

}  // namespace net
}  // namespace base

#endif  // defined(LIBBASE_MODULE_NET)
