#pragma once

#include <utility>

#include "base/bind_internals.h"
#include "base/callback.h"

namespace base {

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

}  // namespace base
