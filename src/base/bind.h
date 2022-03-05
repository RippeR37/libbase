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

template <typename InstanceType>
inline auto Unretained(InstanceType* instance_ptr) {
  return detail::UnretainedType{instance_ptr};
}

template <typename InstanceType>
inline auto RetainedRef(std::shared_ptr<InstanceType> instance_ptr) {
  return detail::RetainedRefType{std::move(instance_ptr)};
}

template <typename InstanceType>
inline auto Owned(std::unique_ptr<InstanceType> instance) {
  return detail::OwnedWrapper<InstanceType>{std::move(instance)};
}

template <typename InstanceType>
inline auto Owned(InstanceType* instance_ptr) {
  return Owned(std::unique_ptr<InstanceType>{instance_ptr});
}

template <typename InstanceType>
inline auto OwnedRef(InstanceType&& instance) {
  return detail::OwnedRefWrapper{std::make_shared<std::decay_t<InstanceType>>(
      std::forward<InstanceType>(instance))};
}

template <typename Functor>
inline auto IgnoreResult(Functor functor) {
  return detail::IgnoreResultType<Functor>{std::forward<Functor>(functor)};
}

}  // namespace base
