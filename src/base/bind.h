#pragma once

#include "base/bind_internals.h"
#include "base/callback.h"
#include "base/type_traits.h"

namespace base {

template <typename Functor, typename... Arguments>
auto BindOnce(Functor&& functor, Arguments&&... arguments) {
  if constexpr (traits::IsCapturelessLambdaV<Functor>) {
    return BindOnce(+functor, std::forward<Arguments>(arguments)...);
  } else {
    return detail::Bind<OnceCallback, false>(
        std::forward<Functor>(functor), std::forward<Arguments>(arguments)...);
  }
}

template <typename Functor, typename... Arguments>
auto BindRepeating(Functor&& functor, Arguments&&... arguments) {
  if constexpr (traits::IsCapturelessLambdaV<Functor>) {
    return BindRepeating(+functor, std::forward<Arguments>(arguments)...);
  } else {
    return detail::Bind<RepeatingCallback, true>(
        std::forward<Functor>(functor), std::forward<Arguments>(arguments)...);
  }
}

}  // namespace base
