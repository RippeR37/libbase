#pragma once

#include "base/type_traits.h"

namespace base {
namespace detail {

template <typename FirstReturnType,
          typename FirstCallback,
          typename SecondCallback,
          typename... FirstArguments>
struct ThenHelper {
  static auto Invoke(FirstCallback first,
                     SecondCallback second,
                     FirstArguments... arguments) {
    if constexpr (std::is_same_v<FirstReturnType, void>) {
      std::move(first).Run(std::forward<FirstArguments>(arguments)...);
      return std::move(second).Run();
    } else {
      return std::move(second).Run(
          std::move(first).Run(std::forward<FirstArguments>(arguments)...));
    }
  }
};

}  // namespace detail
}  // namespace base
