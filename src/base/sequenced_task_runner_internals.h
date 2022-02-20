#pragma once

#include <memory>

namespace base {
namespace detail {

template <typename T>
void DeleteSoonPtr(const T* object) {
  delete object;
}

template <typename T>
void DeleteSoonUniquePtr(T* object) {
  std::default_delete<T>{}(object);
}

}  // namespace detail
}  // namespace base
