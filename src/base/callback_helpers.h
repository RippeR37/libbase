#pragma once

#include <atomic>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"

namespace base {

namespace detail {
template <typename ReturnType, typename... ArgumentTypes>
struct SplitOnceCallbackHelper {
  SplitOnceCallbackHelper(
      base::OnceCallback<ReturnType(ArgumentTypes...)> callback)
      : callback(std::move(callback)) {}

  ReturnType Run(ArgumentTypes... arguments) {
    CHECK(!flag.test_and_set()) << "Split OnceCallback invoked more than once";
    return std::move(callback).Run(std::forward<ArgumentTypes>(arguments)...);
  }

  std::atomic_flag flag{};
  base::OnceCallback<ReturnType(ArgumentTypes...)> callback{};
};
}  // namespace detail

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

template <typename ReturnType, typename... ArgumentTypes>
std::pair<base::OnceCallback<ReturnType(ArgumentTypes...)>,
          base::OnceCallback<ReturnType(ArgumentTypes...)>>
SplitOnceCallback(base::OnceCallback<ReturnType(ArgumentTypes...)> callback) {
  if (!callback) {
    return {};
  }

  using Helper = detail::SplitOnceCallbackHelper<ReturnType, ArgumentTypes...>;
  auto result = base::BindRepeating(
      &Helper::Run, base::Owned(std::make_unique<Helper>(std::move(callback))));
  return {
      static_cast<base::OnceCallback<ReturnType(ArgumentTypes...)>>(result),
      static_cast<base::OnceCallback<ReturnType(ArgumentTypes...)>>(result)};
}

}  // namespace base
