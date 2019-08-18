#pragma once

#include <memory>
#include <type_traits>

namespace base {

namespace detail {

/*
 * Helpers for applying variable ammount of arguments (both already bounded
 * into std::tuple<...> and from variadic template arguments.
 */

template <class FunctionType,
          class BoundArgumentsTupleType,
          std::size_t... Indexes,
          typename... RunArgumentTypes>
constexpr decltype(auto) ApplyBoundAndVariadicArgumentsToFreeFunction(
    FunctionType&& function,
    BoundArgumentsTupleType&& bound_arguments,
    std::index_sequence<Indexes...>,
    RunArgumentTypes&&... run_arguments) {
  return std::invoke(std::forward<FunctionType>(function),
                     std::get<Indexes>(std::forward<BoundArgumentsTupleType>(
                         bound_arguments))...,
                     std::forward<RunArgumentTypes>(run_arguments)...);
}

template <class FunctionType,
          class Class,
          class BoundArgumentsTupleType,
          std::size_t... Indexes,
          typename... RunArgumentTypes>
constexpr decltype(auto) ApplyBoundAndVariadicArgumentsToMemberFunction(
    FunctionType&& function,
    Class&& object,
    BoundArgumentsTupleType&& bound_arguments,
    std::index_sequence<Indexes...>,
    RunArgumentTypes&&... run_arguments) {
  return std::invoke(
      std::forward<FunctionType>(function), std::forward<Class>(object),
      std::get<Indexes>(
          std::forward<BoundArgumentsTupleType>(bound_arguments))...,
      std::forward<RunArgumentTypes>(run_arguments)...);
}

template <class CallbackType,
          class BoundArgumentsTupleType,
          std::size_t... Indexes,
          typename... RunArgumentTypes>
constexpr decltype(auto) ApplyBoundAndVariadicArgumentsToCallback(
    CallbackType&& callback,
    BoundArgumentsTupleType&& bound_arguments,
    std::index_sequence<Indexes...>,
    RunArgumentTypes&&... run_arguments) {
  return std::forward<CallbackType>(callback)->Run(
      std::get<Indexes>(
          std::forward<BoundArgumentsTupleType>(bound_arguments))...,
      std::forward<RunArgumentTypes>(run_arguments)...);
}

/*
 * Callback interfaces.
 */

template <typename ReturnType, typename... ArgumentTypes>
class OnceCallbackInterface {
 public:
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

/*
 * Implementations of OnceCallbackInterface interface.
 */

template <typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class FreeFunctionOnceCallback {
  static_assert(!sizeof(ReturnType), "xxx");
};

template <typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class FreeFunctionOnceCallback<ReturnType,
                               std::tuple<BoundArgumentTypes...>,
                               RunArgumentTypes...>
    : public OnceCallbackInterface<ReturnType, RunArgumentTypes...> {
 public:
  FreeFunctionOnceCallback(ReturnType (*function_ptr)(BoundArgumentTypes...,
                                                      RunArgumentTypes...),
                           std::tuple<BoundArgumentTypes...> bound_arguments)
      : function_ptr_(function_ptr),
        bound_arguments_(std::move(bound_arguments)) {}

  ReturnType Run(RunArgumentTypes... arguments) override {
    return ApplyBoundAndVariadicArgumentsToFreeFunction(
        function_ptr_, std::move(bound_arguments_),
        std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
        std::forward<RunArgumentTypes>(arguments)...);
  }

 private:
  ReturnType (*function_ptr_)(BoundArgumentTypes..., RunArgumentTypes...);
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

template <typename Class,
          typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class MemberFunctionOnceCallback {
  static_assert(!sizeof(Class), "xxx");
};

template <typename Class,
          typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class MemberFunctionOnceCallback<Class,
                                 ReturnType,
                                 std::tuple<BoundArgumentTypes...>,
                                 RunArgumentTypes...>
    : public OnceCallbackInterface<ReturnType, RunArgumentTypes...> {
 public:
  MemberFunctionOnceCallback(
      ReturnType (Class::*member_function_ptr)(BoundArgumentTypes...,
                                               RunArgumentTypes...),
      Class* object,
      std::tuple<BoundArgumentTypes...> bound_arguments)
      : member_function_ptr_(member_function_ptr),
        object_(object),
        bound_arguments_(std::move(bound_arguments)) {}

  ReturnType Run(RunArgumentTypes... arguments) override {
    return ApplyBoundAndVariadicArgumentsToMemberFunction(
        member_function_ptr_, object_, std::move(bound_arguments_),
        std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
        std::forward<RunArgumentTypes>(arguments)...);
  }

 private:
  ReturnType (Class::*member_function_ptr_)(BoundArgumentTypes...,
                                            RunArgumentTypes...);
  Class* object_;
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

template <typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class BoundCallbackOnceCallback {
  static_assert(!sizeof(ReturnType), "");
};

template <typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class BoundCallbackOnceCallback<ReturnType,
                                std::tuple<BoundArgumentTypes...>,
                                RunArgumentTypes...>
    : public OnceCallbackInterface<ReturnType, RunArgumentTypes...> {
 public:
  BoundCallbackOnceCallback(
      std::unique_ptr<OnceCallbackInterface<ReturnType,
                                            BoundArgumentTypes...,
                                            RunArgumentTypes...>> callback,
      std::tuple<BoundArgumentTypes...> bound_arguments)
      : callback_(std::move(callback)),
        bound_arguments_(std::move(bound_arguments)) {}

  ReturnType Run(RunArgumentTypes... run_arguments) override {
    return ApplyBoundAndVariadicArgumentsToCallback(
        std::move(callback_), std::move(bound_arguments_),
        std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
        std::forward<RunArgumentTypes>(run_arguments)...);
  }

 private:
  std::unique_ptr<OnceCallbackInterface<ReturnType,
                                        BoundArgumentTypes...,
                                        RunArgumentTypes...>>
      callback_;
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

/*
 * Implementations of RepeatingCallbackInterface interface.
 */

template <typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class FreeFunctionRepeatingCallback {
  static_assert(!sizeof(ReturnType), "xxx");
};

template <typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class FreeFunctionRepeatingCallback<ReturnType,
                                    std::tuple<BoundArgumentTypes...>,
                                    RunArgumentTypes...>
    : public RepeatingCallbackInterface<ReturnType, RunArgumentTypes...> {
 public:
  FreeFunctionRepeatingCallback(
      ReturnType (*function_ptr)(BoundArgumentTypes..., RunArgumentTypes...),
      std::tuple<BoundArgumentTypes...> bound_arguments)
      : function_ptr_(function_ptr),
        bound_arguments_(std::move(bound_arguments)) {}

  ReturnType Run(RunArgumentTypes... arguments) override {
    return ApplyBoundAndVariadicArgumentsToFreeFunction(
        function_ptr_, bound_arguments_,
        std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
        std::forward<RunArgumentTypes>(arguments)...);
  }

  std::unique_ptr<RepeatingCallbackInterface<ReturnType, RunArgumentTypes...>>
  Clone() const override {
    return std::make_unique<FreeFunctionRepeatingCallback<
        ReturnType, std::tuple<BoundArgumentTypes...>, RunArgumentTypes...>>(
        function_ptr_, bound_arguments_);
  }

 private:
  ReturnType (*function_ptr_)(BoundArgumentTypes..., RunArgumentTypes...);
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

template <typename Class,
          typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class MemberFunctionRepeatingCallback {
  static_assert(!sizeof(Class), "");
};

template <typename Class,
          typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class MemberFunctionRepeatingCallback<Class,
                                      ReturnType,
                                      std::tuple<BoundArgumentTypes...>,
                                      RunArgumentTypes...>
    : public RepeatingCallbackInterface<ReturnType, RunArgumentTypes...> {
 public:
  MemberFunctionRepeatingCallback(
      ReturnType (Class::*member_function_ptr)(BoundArgumentTypes...,
                                               RunArgumentTypes...),
      Class* object,
      std::tuple<BoundArgumentTypes...> bound_arguments)
      : member_function_ptr_(member_function_ptr),
        object_(object),
        bound_arguments_(std::move(bound_arguments)) {}

  ReturnType Run(RunArgumentTypes... arguments) override {
    return ApplyBoundAndVariadicArgumentsToMemberFunction(
        member_function_ptr_, object_, bound_arguments_,
        std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
        std::forward<RunArgumentTypes>(arguments)...);
  }

  std::unique_ptr<RepeatingCallbackInterface<ReturnType, RunArgumentTypes...>>
  Clone() const override {
    return std::make_unique<MemberFunctionRepeatingCallback<
        Class, ReturnType, std::tuple<BoundArgumentTypes...>,
        RunArgumentTypes...>>(member_function_ptr_, object_, bound_arguments_);
  }

 private:
  ReturnType (Class::*member_function_ptr_)(BoundArgumentTypes...,
                                            RunArgumentTypes...);
  Class* object_;
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

template <typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class BoundCallbackRepeatingCallback {
  static_assert(!sizeof(ReturnType), "");
};

template <typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class BoundCallbackRepeatingCallback<ReturnType,
                                     std::tuple<BoundArgumentTypes...>,
                                     RunArgumentTypes...>
    : public RepeatingCallbackInterface<ReturnType, RunArgumentTypes...> {
 public:
  BoundCallbackRepeatingCallback(
      std::unique_ptr<RepeatingCallbackInterface<ReturnType,
                                                 BoundArgumentTypes...,
                                                 RunArgumentTypes...>> callback,
      std::tuple<BoundArgumentTypes...> bound_arguments)
      : callback_(std::move(callback)),
        bound_arguments_(std::move(bound_arguments)) {}

  ReturnType Run(RunArgumentTypes... run_arguments) override {
    return ApplyBoundAndVariadicArgumentsToCallback(
        callback_, bound_arguments_,
        std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
        std::forward<RunArgumentTypes>(run_arguments)...);
  }

  std::unique_ptr<RepeatingCallbackInterface<ReturnType, RunArgumentTypes...>>
  Clone() const override {
    return std::make_unique<BoundCallbackRepeatingCallback<
        ReturnType, std::tuple<BoundArgumentTypes...>, RunArgumentTypes...>>(
        callback_->Clone(), bound_arguments_);
  }

 private:
  std::unique_ptr<RepeatingCallbackInterface<ReturnType,
                                             BoundArgumentTypes...,
                                             RunArgumentTypes...>>
      callback_;
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

}  // namespace detail

/*
 * User-visible OnceCallback<> and RepeatingCallback<> types.
 */

template <typename Signature>
class OnceCallback {
  static_assert(!sizeof(Signature), "xxx");
};

template <typename ReturnType, typename... ArgumentTypes>
class OnceCallback<ReturnType(ArgumentTypes...)> {
 public:
  template <typename... BoundArgumentTypes>
  OnceCallback(ReturnType (*function_ptr)(BoundArgumentTypes...,
                                          ArgumentTypes...),
               std::tuple<BoundArgumentTypes...> bound_arguments =
                   std::tuple<BoundArgumentTypes...>{})
      : impl_(new detail::FreeFunctionOnceCallback<
              ReturnType,
              std::tuple<BoundArgumentTypes...>,
              ArgumentTypes...>(function_ptr, std::move(bound_arguments))) {}

  template <typename Class, typename... BoundArgumentTypes>
  OnceCallback(ReturnType (Class::*member_function_ptr)(BoundArgumentTypes...,
                                                        ArgumentTypes...),
               Class* object,
               std::tuple<BoundArgumentTypes...> bound_arguments =
                   std::tuple<BoundArgumentTypes...>{})
      : impl_(new detail::MemberFunctionOnceCallback<
              Class,
              ReturnType,
              std::tuple<BoundArgumentTypes...>,
              ArgumentTypes...>(member_function_ptr,
                                object,
                                std::move(bound_arguments))) {}

  template <typename... BoundArgumentTypes>
  OnceCallback(OnceCallback<ReturnType(BoundArgumentTypes...,
                                       ArgumentTypes...)>&& callback,
               std::tuple<BoundArgumentTypes...> bound_arguments)
      : impl_(new detail::BoundCallbackOnceCallback<
              ReturnType,
              std::tuple<BoundArgumentTypes...>,
              ArgumentTypes...>(std::move(callback.impl_),
                                std::move(bound_arguments))) {}

  operator bool() const { return !!impl_; }

  ReturnType Run(ArgumentTypes... arguments) && {
    OnceCallback callback = std::move(*this);
    return callback.impl_->Run(std::forward<ArgumentTypes>(arguments)...);
  }

 private:
  template <typename FuncSig>
  friend class OnceCallback;

  template <typename FuncSig>
  friend class RepeatingCallback;

  OnceCallback(
      std::unique_ptr<
          detail::OnceCallbackInterface<ReturnType, ArgumentTypes...>> impl)
      : impl_(std::move(impl)) {}

  std::unique_ptr<detail::OnceCallbackInterface<ReturnType, ArgumentTypes...>>
      impl_;
};

//
//
//

template <typename Signature>
class RepeatingCallback {
  static_assert(!sizeof(Signature), "xxx");
};

template <typename ReturnType, typename... ArgumentTypes>
class RepeatingCallback<ReturnType(ArgumentTypes...)> {
 public:
  template <typename... BoundArgumentTypes>
  RepeatingCallback(ReturnType (*func)(BoundArgumentTypes..., ArgumentTypes...),
                    std::tuple<BoundArgumentTypes...> bound_arguments =
                        std::tuple<BoundArgumentTypes...>{})
      : impl_(new detail::FreeFunctionRepeatingCallback<
              ReturnType,
              std::tuple<BoundArgumentTypes...>,
              ArgumentTypes...>(func, std::move(bound_arguments))) {}

  template <typename Class, typename... BoundArgumentTypes>
  RepeatingCallback(ReturnType (Class::*member_func)(BoundArgumentTypes...,
                                                     ArgumentTypes...),
                    Class* obj,
                    std::tuple<BoundArgumentTypes...> bound_arguments =
                        std::tuple<BoundArgumentTypes...>{})
      : impl_(new detail::MemberFunctionRepeatingCallback<
              Class,
              ReturnType,
              std::tuple<BoundArgumentTypes...>,
              ArgumentTypes...>(member_func, obj, std::move(bound_arguments))) {
  }

  template <typename... BoundArgumentTypes>
  RepeatingCallback(
      const RepeatingCallback<ReturnType(BoundArgumentTypes...,
                                         ArgumentTypes...)>& callback,
      std::tuple<BoundArgumentTypes...> bound_arguments)
      : impl_(new detail::BoundCallbackRepeatingCallback<
              ReturnType,
              std::tuple<BoundArgumentTypes...>,
              ArgumentTypes...>(callback.impl_->Clone(),
                                std::move(bound_arguments))) {}

  operator OnceCallback<ReturnType(ArgumentTypes...)>() const {
    return OnceCallback<ReturnType(ArgumentTypes...)>{impl_->Clone()};
  }

  operator bool() const { return !!impl_; }

  ReturnType Run(ArgumentTypes... arguments) const& {
    return impl_->Run(std::forward<ArgumentTypes>(arguments)...);
  }

  ReturnType Run(ArgumentTypes... arguments) && {
    RepeatingCallback callback = std::move(*this);
    callback->Run(std::forward<ArgumentTypes>(arguments)...);
  }

 private:
  template <typename FuncSig>
  friend class RepeatingCallback;

  std::unique_ptr<
      detail::RepeatingCallbackInterface<ReturnType, ArgumentTypes...>>
      impl_;
};

}  // namespace base
