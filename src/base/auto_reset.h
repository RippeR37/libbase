#pragma once

#include <utility>

namespace base {

template <typename T>
class AutoReset {
 public:
  template <typename U>
  AutoReset(T* variable, U&& new_value)
      : variable_(variable),
        original_value_(std::exchange(*variable_, std::forward<U>(new_value))) {
  }
  ~AutoReset() { *variable_ = std::move(original_value_); }

  AutoReset(AutoReset&&) = delete;
  AutoReset& operator=(AutoReset&&) = delete;

 private:
  T* const variable_;
  T original_value_;
};

}  // namespace base
