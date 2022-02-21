#pragma once

#include <utility>

#include "base/bind_internals.h"
#include "base/callback.h"

namespace base {

//
// Bind callbacks
//

template <typename Functor, typename... Arguments>
auto BindOnce(Functor&& functor, Arguments&&... arguments) {
  return detail::Bind<OnceCallback, false>(
      std::forward<Functor>(functor), std::forward<Arguments>(arguments)...);
}

template <typename Functor, typename... Arguments>
auto BindRepeating(Functor&& functor, Arguments&&... arguments) {
  return detail::Bind<RepeatingCallback, true>(
      std::forward<Functor>(functor), std::forward<Arguments>(arguments)...);
}

//
// Helpers
//

template <typename Functor>
inline auto IgnoreResult(Functor functor) {
  return detail::IgnoreResultType<Functor>{std::forward<Functor>(functor)};
}

}  // namespace base
