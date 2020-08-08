#pragma once

#include "base/bind.h"

namespace base {

class DoNothing {
 public:
  template <typename... Args>
  static OnceCallback<void(Args...)> Once() {
    return BindOnce([](Args... /*args*/) {});
  }

  template <typename... Args>
  static RepeatingCallback<void(Args...)> Repeatedly() {
    return BindRepeating([](Args... /*args*/) {});
  }

  template <typename... Args>
  operator OnceCallback<void(Args...)>() {
    return Once<Args...>();
  }

  template <typename... Args>
  operator RepeatingCallback<void(Args...)>() {
    return Repeatedly<Args...>();
  }
};

}  // namespace base
