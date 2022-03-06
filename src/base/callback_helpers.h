#pragma once

#include "base/bind.h"
#include "base/callback.h"

namespace base {

class ScopedClosureRunner {
 public:
  ScopedClosureRunner();
  explicit ScopedClosureRunner(OnceClosure closure);
  ScopedClosureRunner(ScopedClosureRunner&&);
  ~ScopedClosureRunner();

  ScopedClosureRunner& operator=(ScopedClosureRunner&& other);

  explicit operator bool() const;

  void RunAndReset();
  void ReplaceClosure(OnceClosure new_closure);
  OnceClosure Release();

 private:
  OnceClosure closure_;
};

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
