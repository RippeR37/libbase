#pragma once

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "base/callback.h"
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
  if constexpr (std::is_member_function_pointer_v<
                    traits::RemoveCVRefT<FunctionType>>) {
    if constexpr (sizeof...(Indexes) > 0) {
      if constexpr (!std::is_pointer_v<
                        std::tuple_element_t<0, BoundArgumentsTupleType>>) {
        if (!std::get<0>(bound_arguments)) {
          return;
        }
      }
    }
  }
  return std::invoke(std::forward<FunctionType>(function),
                     std::get<Indexes>(std::forward<BoundArgumentsTupleType>(
                         bound_arguments))...,
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
  return std::forward<CallbackType>(callback).Run(
      std::get<Indexes>(
          std::forward<BoundArgumentsTupleType>(bound_arguments))...,
      std::forward<RunArgumentTypes>(run_arguments)...);
}

//
// Functor traits
//

template <typename ReturnTypeParam,
          typename ArgumentsTypeParam,
          size_t ArgumentsCountParam>
struct FunctorTraitsImpl {
  using ReturnType = ReturnTypeParam;
  using ArgumentsType = ArgumentsTypeParam;
  static constexpr size_t ArgumentsCount = ArgumentsCountParam;
};

template <typename Functor, typename InstancePointer, typename = void>
struct FunctorTraits {
  static_assert(!sizeof(Functor), "Invalid instantiation");
};

template <typename FunctorType>
struct IgnoreResultType {
  template <typename... ArgumentsType>
  void operator()(ArgumentsType&&... arguments) {
    static_cast<void>(
        std::invoke(functor, std::forward<ArgumentsType>(arguments)...));
  }

  FunctorType functor;
};

// Function
template <typename ReturnType,
          typename... ArgumentTypes,
          typename InstancePointer>
struct FunctorTraits<ReturnType (*)(ArgumentTypes...), InstancePointer>
    : FunctorTraitsImpl<ReturnType,
                        std::tuple<ArgumentTypes...>,
                        sizeof...(ArgumentTypes)> {};

// Member functions
#define LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(...)                       \
  template <typename ReturnType, typename ClassType,                         \
            typename InstancePointerType, typename... ArgumentTypes>         \
  struct FunctorTraits<ReturnType (ClassType::*)(ArgumentTypes...)           \
                           __VA_ARGS__,                                      \
                       InstancePointerType>                                  \
      : FunctorTraitsImpl<ReturnType,                                        \
                          std::tuple<InstancePointerType, ArgumentTypes...>, \
                          1 + sizeof...(ArgumentTypes)> {}

LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT();
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(volatile);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const volatile);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(&);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const&);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(volatile&);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const volatile&);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(&&);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const&&);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(volatile&&);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const volatile&&);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(volatile noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const volatile noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(&noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const& noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(volatile& noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const volatile& noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(&&noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const&& noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(volatile&& noexcept);
LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT(const volatile&& noexcept);

#undef LIBBASE_IMPL_CREATE_MEMBER_FUNCTION_TRAIT

// Callbacks
template <typename ReturnType,
          typename... ArgumentTypes,
          typename InstancePointer>
struct FunctorTraits<::base::OnceCallback<ReturnType(ArgumentTypes...)>,
                     InstancePointer>
    : FunctorTraitsImpl<ReturnType,
                        std::tuple<ArgumentTypes...>,
                        sizeof...(ArgumentTypes)> {};

template <typename ReturnType,
          typename... ArgumentTypes,
          typename InstancePointer>
struct FunctorTraits<::base::RepeatingCallback<ReturnType(ArgumentTypes...)>,
                     InstancePointer>
    : FunctorTraitsImpl<ReturnType,
                        std::tuple<ArgumentTypes...>,
                        sizeof...(ArgumentTypes)> {};

// Lambda
template <typename LambdaType, typename InstancePointer>
struct FunctorTraits<
    LambdaType,
    InstancePointer,
    std::enable_if_t<traits::IsCapturelessLambdaV<LambdaType>, void>> {
  using FunctionPointerType = decltype(+std::declval<LambdaType>());
  using FunctionPointerTraits =
      FunctorTraits<FunctionPointerType, InstancePointer>;

  using ReturnType = typename FunctionPointerTraits::ReturnType;
  using ArgumentsType = typename FunctionPointerTraits::ArgumentsType;
  static constexpr size_t ArgumentsCount =
      FunctionPointerTraits::ArgumentsCount;
};

// IgnoreResult
template <typename Functor, typename InstancePointer>
struct FunctorTraits<IgnoreResultType<Functor>, InstancePointer> {
  using Traits = FunctorTraits<Functor, InstancePointer>;

  using ReturnType = void;
  using ArgumentsType = typename Traits::ArgumentsType;
  static constexpr size_t ArgumentsCount = Traits::ArgumentsCount;
};

//
// Callback implementation
//

template <typename DerivedType, typename ReturnType, typename... ArgumentTypes>
class RepeatingCallbackCrtpImpl
    : public RepeatingCallbackInterface<ReturnType, ArgumentTypes...> {
 public:
  std::unique_ptr<RepeatingCallbackInterface<ReturnType, ArgumentTypes...>>
  Clone() const override {
    const auto* this_derived = static_cast<const DerivedType*>(this);
    return std::make_unique<DerivedType>(*this_derived);
  }
};

template <bool is_repeating,
          typename FunctorType,
          typename ReturnType,
          typename BoundArgumentTupleType,
          typename RunArgumentTypes>
class FunctorCallback {
  static_assert(!sizeof(ReturnType), "Invalid template instantiation");
};

template <bool is_repeating,
          typename FunctorType,
          typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class FunctorCallback<is_repeating,
                      FunctorType,
                      ReturnType,
                      std::tuple<BoundArgumentTypes...>,
                      std::tuple<RunArgumentTypes...>>
    : public std::conditional_t<
          is_repeating,
          RepeatingCallbackCrtpImpl<
              FunctorCallback<is_repeating,
                              FunctorType,
                              ReturnType,
                              std::tuple<BoundArgumentTypes...>,
                              std::tuple<RunArgumentTypes...>>,
              ReturnType,
              RunArgumentTypes...>,
          OnceCallbackInterface<ReturnType, RunArgumentTypes...>> {
 public:
  using RawFunctorType = traits::RemoveCVRefT<FunctorType>;

  FunctorCallback(FunctorType&& functor,
                  std::tuple<BoundArgumentTypes...>&& bound_arguments)
      : functor_(std::forward<FunctorType>(functor)),
        bound_arguments_(std::move(bound_arguments)) {}

  ReturnType Run(RunArgumentTypes... arguments) override {
    if constexpr (traits::IsOnceCallbackV<RawFunctorType>) {
      return ApplyBoundAndVariadicArgumentsToCallback(
          std::move(functor_), std::move(bound_arguments_),
          std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
          std::forward<RunArgumentTypes>(arguments)...);
    } else if constexpr (traits::IsRepeatingCallbackV<RawFunctorType>) {
      return ApplyBoundAndVariadicArgumentsToCallback(
          functor_, bound_arguments_,
          std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
          std::forward<RunArgumentTypes>(arguments)...);
    } else {
      return ApplyBoundAndVariadicArgumentsToFreeFunction(
          functor_, std::move(bound_arguments_),
          std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
          std::forward<RunArgumentTypes>(arguments)...);
    }
  }

 private:
  RawFunctorType functor_;
  std::tuple<BoundArgumentTypes...> bound_arguments_;
};

template <template <typename> class CallbackType,
          bool is_repeating,
          typename Functor,
          typename... Arguments>
auto Bind(Functor&& functor, Arguments&&... arguments) {
  constexpr bool has_arguments = sizeof...(Arguments) > 0;
  using FirstArgumentType = std::conditional_t<
      has_arguments, std::tuple_element_t<0, std::tuple<Arguments..., void>>,
      void>;
  using FunctorTraits = FunctorTraits<traits::RemoveCVRefT<Functor>,
                                      traits::RemoveCVRefT<FirstArgumentType>>;

  constexpr size_t func_arg_cnt = FunctorTraits::ArgumentsCount;
  constexpr size_t bind_arg_cnt = sizeof...(Arguments);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  // 1. Bind arguments
  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt,
                              typename FunctorTraits::ArgumentsType>;
  BoundArgumentsType bound_args{std::forward<Arguments>(arguments)...};

  // 2. Compute values and types of the result
  constexpr size_t remaining_arg_cnt =
      func_arg_cnt - std::min(bind_arg_cnt, func_arg_cnt);
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, remaining_arg_cnt,
                          typename FunctorTraits::ArgumentsType>;
  using ResultType =
      BindResultType<CallbackType, typename FunctorTraits::ReturnType,
                     RemainingArgumentsType>;

  // 3. Create the callback object.
  return BindAccessHelper::Create<ResultType>(
      std::make_unique<FunctorCallback<
          is_repeating, Functor, typename FunctorTraits::ReturnType,
          BoundArgumentsType, RemainingArgumentsType>>(
          std::forward<Functor>(functor), std::move(bound_args)));
}

}  // namespace detail
}  // namespace base
