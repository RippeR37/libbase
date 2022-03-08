#pragma once

#include <mutex>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/type_traits.h"

namespace base {

namespace detail {
template <typename ElementType, typename CallbackArgumentsType>
class BarrierCallbackHelper {
 public:
  BarrierCallbackHelper(size_t required_run_count,
                        OnceCallback<void(CallbackArgumentsType)> callback)
      : required_run_count_(required_run_count),
        callback_(std::move(callback)) {}

  void Run(ElementType element) {
    bool is_last = false;
    {
      std::unique_lock guard{mutex_};
      arguments_.push_back(std::forward<ElementType>(element));
      DCHECK_GE(required_run_count_, 0u);
      is_last = (--required_run_count_ == 0);
    }
    if (is_last) {
      std::move(callback_).Run(std::exchange(arguments_, {}));
    }
  }

 private:
  using ArgumentsStorageType = traits::RemoveCVRefT<CallbackArgumentsType>;

  std::mutex mutex_;
  ArgumentsStorageType arguments_;
  std::size_t required_run_count_;
  OnceCallback<void(CallbackArgumentsType)> callback_;
};
}  // namespace detail

template <typename ElementType, typename CallbackArgumentsType>
RepeatingCallback<void(ElementType)> BarrierCallback(
    size_t required_run_count,
    OnceCallback<void(CallbackArgumentsType)> callback) {
  if (required_run_count == 0) {
    std::move(callback).Run(CallbackArgumentsType{});
    return {};
  }

  using Helper =
      detail::BarrierCallbackHelper<ElementType, CallbackArgumentsType>;
  return BindRepeating(
      &Helper::Run,
      Owned(std::make_unique<Helper>(required_run_count, std::move(callback))));
}

template <typename ElementType, typename CallbackArgumentsType>
RepeatingCallback<void(ElementType)> BarrierCallback(
    size_t required_run_count,
    RepeatingCallback<void(CallbackArgumentsType)> callback) {
  return BarrierCallback(
      required_run_count,
      OnceCallback<void(CallbackArgumentsType)>{std::move(callback)});
}

}  // namespace base
