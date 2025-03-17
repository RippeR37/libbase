#pragma once

#include <memory>

namespace base {
namespace detail {

template <typename ReturnType, typename... ArgumentTypes>
class OnceCallbackInterface {
 public:
  OnceCallbackInterface() = default;
  OnceCallbackInterface(const OnceCallbackInterface&) = default;
  virtual ~OnceCallbackInterface() = default;

  virtual ReturnType Run(ArgumentTypes...) = 0;
};

template <typename ReturnType, typename... ArgumentTypes>
class RepeatingCallbackInterface
    : public OnceCallbackInterface<ReturnType, ArgumentTypes...> {
 public:
  virtual std::unique_ptr<
      RepeatingCallbackInterface<ReturnType, ArgumentTypes...>>
  Clone() const = 0;
};

}  // namespace detail
}  // namespace base
