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
// Helpers for invoking member functions with different instance-pointer types
//

template <typename InstanceType>
struct UnretainedType {
  UnretainedType(InstanceType* instance_ptr) : instance_ptr(instance_ptr) {}
  InstanceType* instance_ptr;
};

template <typename InstanceType>
struct RetainedRefType {
  RetainedRefType(std::shared_ptr<InstanceType> instance_ptr)
      : instance_ptr(std::move(instance_ptr)) {}
  std::shared_ptr<InstanceType> instance_ptr;
};

template <typename Functor, typename Instance, typename... ArgumentTypes>
static constexpr decltype(auto) MemberFunctionInvoke(
    Functor&& functor,
    const UnretainedType<Instance>& instance,
    ArgumentTypes&&... arguments) {
  return std::invoke(std::forward<Functor>(functor), instance.instance_ptr,
                     std::forward<ArgumentTypes>(arguments)...);
}

template <typename Functor, typename Instance, typename... ArgumentTypes>
static constexpr decltype(auto) MemberFunctionInvoke(
    Functor&& functor,
    const RetainedRefType<Instance>& instance,
    ArgumentTypes&&... arguments) {
  return std::invoke(std::forward<Functor>(functor), instance.instance_ptr,
                     std::forward<ArgumentTypes>(arguments)...);
}

template <typename Functor, typename Class, typename... ArgumentTypes>
inline constexpr void MemberFunctionInvoke(Functor&& functor,
                                           const base::WeakPtr<Class>& weak_ptr,
                                           ArgumentTypes&&... arguments) {
  if (weak_ptr) {
    std::invoke(std::forward<Functor>(functor), weak_ptr,
                std::forward<ArgumentTypes>(arguments)...);
  }
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

  template <typename Functor,
            std::size_t... Indexes,
            typename BoundArgumentsTupleType,
            typename... RunArgumentTypes>
  static constexpr decltype(auto) Invoke(
      Functor&& functor,
      std::index_sequence<Indexes...>,
      BoundArgumentsTupleType&& bound_arguments,
      RunArgumentTypes&&... run_arguments) {
    return std::invoke(std::forward<Functor>(functor),
                       std::get<Indexes>(std::forward<BoundArgumentsTupleType>(
                           bound_arguments))...,
                       std::forward<RunArgumentTypes>(run_arguments)...);
  }
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
                          1 + sizeof...(ArgumentTypes)> {                    \
    template <typename Functor,                                              \
              std::size_t... Indexes,                                        \
              typename BoundArgumentsTupleType,                              \
              typename... RunArgumentTypes>                                  \
    static constexpr decltype(auto) Invoke(                                  \
        Functor&& functor,                                                   \
        std::index_sequence<Indexes...>,                                     \
        BoundArgumentsTupleType&& bound_arguments,                           \
        RunArgumentTypes&&... run_arguments) {                               \
      return MemberFunctionInvoke(                                           \
          std::forward<Functor>(functor),                                    \
          std::get<Indexes>(                                                 \
              std::forward<BoundArgumentsTupleType>(bound_arguments))...,    \
          std::forward<RunArgumentTypes>(run_arguments)...);                 \
    }                                                                        \
  }

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
                        sizeof...(ArgumentTypes)> {
  template <typename Functor,
            std::size_t... Indexes,
            typename BoundArgumentsTupleType,
            typename... RunArgumentTypes>
  static constexpr decltype(auto) Invoke(
      Functor&& functor,
      std::index_sequence<Indexes...>,
      BoundArgumentsTupleType&& bound_arguments,
      RunArgumentTypes&&... run_arguments) {
    return std::forward<Functor>(functor).Run(
        std::get<Indexes>(
            std::forward<BoundArgumentsTupleType>(bound_arguments))...,
        std::forward<RunArgumentTypes>(run_arguments)...);
  }
};

template <typename ReturnType,
          typename... ArgumentTypes,
          typename InstancePointer>
struct FunctorTraits<::base::RepeatingCallback<ReturnType(ArgumentTypes...)>,
                     InstancePointer>
    : FunctorTraitsImpl<ReturnType,
                        std::tuple<ArgumentTypes...>,
                        sizeof...(ArgumentTypes)> {
  template <typename Functor,
            std::size_t... Indexes,
            typename BoundArgumentsTupleType,
            typename... RunArgumentTypes>
  static constexpr decltype(auto) Invoke(
      const Functor& functor,
      std::index_sequence<Indexes...>,
      const BoundArgumentsTupleType& bound_arguments,
      RunArgumentTypes&&... run_arguments) {
    return functor.Run(std::get<Indexes>(bound_arguments)...,
                       std::forward<RunArgumentTypes>(run_arguments)...);
  }
};

// Lambda
template <typename LambdaType, typename InstancePointer>
struct FunctorTraits<
    LambdaType,
    InstancePointer,
    std::enable_if_t<traits::IsCapturelessLambdaV<LambdaType>, void>>
    : FunctorTraits<decltype(+std::declval<LambdaType>()), InstancePointer> {};

// IgnoreResult
template <typename Functor, typename InstancePointer>
struct FunctorTraits<IgnoreResultType<Functor>, InstancePointer>
    : FunctorTraits<Functor, InstancePointer> {
  using ReturnType = void;
};

//
// Wrap bound types customization
//

template <typename FunctorArgument, typename BindArgument>
struct WrapHelper {
  template <typename T>
  static auto Wrap(T&& arg) {
    return FunctorArgument{std::forward<T>(arg)};
  }

  static constexpr bool IsWrapped = false;
};

template <typename FunctorArgument, typename BindArgument>
struct WrapHelper<FunctorArgument, std::reference_wrapper<BindArgument>> {
  template <typename T>
  static auto Wrap(T&& arg) {
    return std::reference_wrapper<FunctorArgument>{std::forward<T>(arg)};
  }

  static constexpr bool IsWrapped = true;
};

template <typename T, typename U>
struct WrapTypesHelper {
  static_assert(!sizeof(T), "Invalid instantiation");
};

template <typename... FunctorTypes, typename... ArgTypes>
struct WrapTypesHelper<std::tuple<FunctorTypes...>, std::tuple<ArgTypes...>> {
  using ResultType =
      std::tuple<decltype(WrapHelper<traits::RemoveCVRefT<FunctorTypes>,
                                     traits::RemoveCVRefT<ArgTypes>>::
                              Wrap(std::declval<ArgTypes>()))...>;

  static ResultType WrapArguments(ArgTypes&&... args) {
    constexpr bool cannot_bind_value_to_nonconst_reference_test =
        ((!std::is_reference_v<FunctorTypes> ||
          std::is_const_v<std::remove_reference_t<FunctorTypes>> ||
          WrapHelper<traits::RemoveCVRefT<FunctorTypes>,
                     traits::RemoveCVRefT<ArgTypes>>::IsWrapped)

         && ...);
    static_assert(cannot_bind_value_to_nonconst_reference_test,
                  "Cannot bind unwrapped value to non-const reference");

    return ResultType{WrapHelper<
        traits::RemoveCVRefT<FunctorTypes>,
        traits::RemoveCVRefT<ArgTypes>>::Wrap(std::forward<ArgTypes>(args))...};
  }
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
          typename FunctorTraits,
          typename FunctorType,
          typename ReturnType,
          typename BoundArgumentTupleType,
          typename RunArgumentTypes>
class FunctorCallback {
  static_assert(!sizeof(ReturnType), "Invalid template instantiation");
};

template <bool is_repeating,
          typename FunctorTraits,
          typename FunctorType,
          typename ReturnType,
          typename... BoundArgumentTypes,
          typename... RunArgumentTypes>
class FunctorCallback<is_repeating,
                      FunctorTraits,
                      FunctorType,
                      ReturnType,
                      std::tuple<BoundArgumentTypes...>,
                      std::tuple<RunArgumentTypes...>>
    : public std::conditional_t<
          is_repeating,
          RepeatingCallbackCrtpImpl<
              FunctorCallback<is_repeating,
                              FunctorTraits,
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
    if constexpr (is_repeating) {
      return FunctorTraits::Invoke(
          functor_, std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
          bound_arguments_, std::forward<RunArgumentTypes>(arguments)...);
    } else {
      return FunctorTraits::Invoke(
          std::move(functor_),
          std::make_index_sequence<sizeof...(BoundArgumentTypes)>{},
          std::move(bound_arguments_),
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
  using FunctorHeadTypes =
      traits::HeadTypesRangeT<bind_arg_cnt,
                              typename FunctorTraits::ArgumentsType>;
  using BoundArgumentsType =
      typename WrapTypesHelper<FunctorHeadTypes,
                               std::tuple<Arguments...>>::ResultType;
  auto bound_args =
      WrapTypesHelper<FunctorHeadTypes, std::tuple<Arguments...>>::
          WrapArguments(std::forward<Arguments>(arguments)...);

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
      std::make_unique<
          FunctorCallback<is_repeating, FunctorTraits, Functor,
                          typename FunctorTraits::ReturnType,
                          BoundArgumentsType, RemainingArgumentsType>>(
          std::forward<Functor>(functor), std::move(bound_args)));
}

}  // namespace detail
}  // namespace base
