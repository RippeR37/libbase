#pragma once

#include <cstddef>
#include <functional>
#include <tuple>
#include <utility>

#include "base/callback_iface.h"
#include "base/memory/weak_ptr.h"
#include "base/type_traits.h"

namespace base {
namespace detail {

//
// Helpers to retrieve BindOnce()/BindRepeating() result type.
//

template <template <typename> class CallbackType,
          typename ReturnType,
          typename ArgumentsTupleType>
struct BindResult {};

template <template <typename> class CallbackType,
          typename ReturnType,
          typename... ArgumentTypes>
struct BindResult<CallbackType, ReturnType, std::tuple<ArgumentTypes...>> {
  using type = CallbackType<ReturnType(ArgumentTypes...)>;
};

template <template <typename> class CallbackType,
          typename ReturnType,
          typename... ArgumentTypes>
using BindResultType =
    typename BindResult<CallbackType, ReturnType, ArgumentTypes...>::type;

//
// Helpers to access private CTOR or |impl_| from Callback.
//

class BindAccessHelper {
 public:
  template <typename CallbackType, typename ImplType>
  static CallbackType Create(ImplType impl) {
    return CallbackType{std::move(impl)};
  }

  template <typename CallbackType>
  static auto ExtractImpl(CallbackType&& callback) {
    return std::move(callback.impl_);
  }

  template <typename CallbackType>
  static auto CloneImpl(const CallbackType& callback) {
    if (callback) {
      return callback.impl_->Clone();
    }
    return decltype(callback.impl_->Clone()){};
  }
};

//
// Helpers for applying variable ammount of arguments (both already bounded
// into std::tuple<...> and from variadic template arguments.
//

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

//
// Implementations of OnceCallbackInterface.
//

template <typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class FreeFunctionOnceCallback {
  static_assert(!sizeof(ReturnType), "Invalid template instantiation");
};

template <typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class FreeFunctionOnceCallback<ReturnType,
                               std::tuple<BoundArgumentTypes...>,
                               std::tuple<RunArgumentTypes...>>
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
  static_assert(!sizeof(Class), "Invalid template instantiation");
};

template <typename Class,
          typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class MemberFunctionOnceCallback<Class,
                                 ReturnType,
                                 std::tuple<BoundArgumentTypes...>,
                                 std::tuple<RunArgumentTypes...>>
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

template <typename Class,
          typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class MemberFunctionWeakPtrOnceCallback {
  static_assert(!sizeof(Class), "Invalid template instantiation");
};

template <typename Class,
          typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class MemberFunctionWeakPtrOnceCallback<Class,
                                        ReturnType,
                                        std::tuple<BoundArgumentTypes...>,
                                        std::tuple<RunArgumentTypes...>>
    : public OnceCallbackInterface<ReturnType, RunArgumentTypes...> {
 public:
  static_assert(std::is_same_v<ReturnType, void>,
                "Cannot bind function with return value to base::WeakPtr");

  MemberFunctionWeakPtrOnceCallback(
      ReturnType (Class::*member_function_ptr)(BoundArgumentTypes...,
                                               RunArgumentTypes...),
      WeakPtr<Class> object,
      std::tuple<BoundArgumentTypes...> bound_arguments)
      : member_function_ptr_(member_function_ptr),
        object_(std::move(object)),
        bound_arguments_(std::move(bound_arguments)) {}

  ReturnType Run(RunArgumentTypes... arguments) override {
    if (object_) {
      return ApplyBoundAndVariadicArgumentsToMemberFunction(
          member_function_ptr_, object_.Get(), std::move(bound_arguments_),
          std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
          std::forward<RunArgumentTypes>(arguments)...);
    }
  }

 private:
  ReturnType (Class::*member_function_ptr_)(BoundArgumentTypes...,
                                            RunArgumentTypes...);
  WeakPtr<Class> object_;
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

template <typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class BoundCallbackOnceCallback {
  static_assert(!sizeof(ReturnType), "Invalid template instantiation");
};

template <typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class BoundCallbackOnceCallback<ReturnType,
                                std::tuple<BoundArgumentTypes...>,
                                std::tuple<RunArgumentTypes...>>
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

//
// Implementations of RepeatingCallbackInterface.
//

template <typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class FreeFunctionRepeatingCallback {
  static_assert(!sizeof(ReturnType), "Invalid template instantiation");
};

template <typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class FreeFunctionRepeatingCallback<ReturnType,
                                    std::tuple<BoundArgumentTypes...>,
                                    std::tuple<RunArgumentTypes...>>
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
        ReturnType, std::tuple<BoundArgumentTypes...>,
        std::tuple<RunArgumentTypes...>>>(function_ptr_, bound_arguments_);
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
  static_assert(!sizeof(Class), "Invalid template instantiation");
};

template <typename Class,
          typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class MemberFunctionRepeatingCallback<Class,
                                      ReturnType,
                                      std::tuple<BoundArgumentTypes...>,
                                      std::tuple<RunArgumentTypes...>>
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
        std::tuple<RunArgumentTypes...>>>(member_function_ptr_, object_,
                                          bound_arguments_);
  }

 private:
  ReturnType (Class::*member_function_ptr_)(BoundArgumentTypes...,
                                            RunArgumentTypes...);
  Class* object_;
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

template <typename Class,
          typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class MemberFunctionWeakPtrRepeatingCallback {
  static_assert(!sizeof(Class), "Invalid template instantiation");
};

template <typename Class,
          typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class MemberFunctionWeakPtrRepeatingCallback<Class,
                                             ReturnType,
                                             std::tuple<BoundArgumentTypes...>,
                                             std::tuple<RunArgumentTypes...>>
    : public RepeatingCallbackInterface<ReturnType, RunArgumentTypes...> {
 public:
  static_assert(std::is_same_v<ReturnType, void>,
                "Cannot bind function with return value to base::WeakPtr");

  MemberFunctionWeakPtrRepeatingCallback(
      ReturnType (Class::*member_function_ptr)(BoundArgumentTypes...,
                                               RunArgumentTypes...),
      WeakPtr<Class> object,
      std::tuple<BoundArgumentTypes...> bound_arguments)
      : member_function_ptr_(member_function_ptr),
        object_(std::move(object)),
        bound_arguments_(std::move(bound_arguments)) {}

  ReturnType Run(RunArgumentTypes... arguments) override {
    if (object_) {
      return ApplyBoundAndVariadicArgumentsToMemberFunction(
          member_function_ptr_, object_.Get(), bound_arguments_,
          std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
          std::forward<RunArgumentTypes>(arguments)...);
    }
  }

  std::unique_ptr<RepeatingCallbackInterface<ReturnType, RunArgumentTypes...>>
  Clone() const override {
    return std::make_unique<MemberFunctionWeakPtrRepeatingCallback<
        Class, ReturnType, std::tuple<BoundArgumentTypes...>,
        std::tuple<RunArgumentTypes...>>>(member_function_ptr_, object_,
                                          bound_arguments_);
  }

 private:
  ReturnType (Class::*member_function_ptr_)(BoundArgumentTypes...,
                                            RunArgumentTypes...);
  WeakPtr<Class> object_;
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

template <typename ReturnType,
          typename BoundArgumentsTupleType,
          typename... RunArgumentTypes>
class BoundCallbackRepeatingCallback {
  static_assert(!sizeof(ReturnType), "Invalid template instantiation");
};

template <typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class BoundCallbackRepeatingCallback<ReturnType,
                                     std::tuple<BoundArgumentTypes...>,
                                     std::tuple<RunArgumentTypes...>>
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
        ReturnType, std::tuple<BoundArgumentTypes...>,
        std::tuple<RunArgumentTypes...>>>(callback_->Clone(), bound_arguments_);
  }

 private:
  std::unique_ptr<RepeatingCallbackInterface<ReturnType,
                                             BoundArgumentTypes...,
                                             RunArgumentTypes...>>
      callback_;
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

}  // namespace detail
}  // namespace base
