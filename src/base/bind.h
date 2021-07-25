#pragma once

#include "base/bind_internals.h"
#include "base/callback.h"
#include "base/type_traits.h"

namespace base {

//
// BindOnce()
//

template <typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindOnce(ReturnType (*function_ptr)(FunctionArgumentTypes...),
              BindArgumentTypes&&... bind_args) {
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  // This is a workaround for a Clang bug. If |bind_arg_cnt > func_arg_cnt|,
  // then even though the above assert will fail, Clang will still process the
  // below typedefs which might instantiate std::index_sequence with huge count
  // which can crash the compiler.
  constexpr size_t remaining_arg_count =
      func_arg_cnt - std::min(bind_arg_cnt, func_arg_cnt);

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, remaining_arg_count,
                          FunctionArgumentTypes...>;
  using ResultType =
      detail::BindResultType<OnceCallback, ReturnType, RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};

  return detail::BindAccessHelper::Create<ResultType>(
      std::make_unique<detail::FreeFunctionOnceCallback<
          ReturnType, BoundArgumentsType, RemainingArgumentsType>>(
          function_ptr, std::move(bounded_args)));
}

template <typename Class,
          typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindOnce(
    ReturnType (Class::*member_function_ptr)(FunctionArgumentTypes...),
    Class* object,
    BindArgumentTypes&&... bind_args) {
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  // This is a workaround for a Clang bug. If |bind_arg_cnt > func_arg_cnt|,
  // then even though the above assert will fail, Clang will still process the
  // below typedefs which might instantiate std::index_sequence with huge count
  // which can crash the compiler.
  constexpr size_t remaining_arg_count =
      func_arg_cnt - std::min(bind_arg_cnt, func_arg_cnt);

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, remaining_arg_count,
                          FunctionArgumentTypes...>;
  using ResultType =
      detail::BindResultType<OnceCallback, ReturnType, RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return detail::BindAccessHelper::Create<ResultType>(
      std::make_unique<detail::MemberFunctionOnceCallback<
          Class, ReturnType, BoundArgumentsType, RemainingArgumentsType>>(
          member_function_ptr, object, std::move(bounded_args)));
}

template <typename Class,
          typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindOnce(
    ReturnType (Class::*member_function_ptr)(FunctionArgumentTypes...),
    WeakPtr<Class> object,
    BindArgumentTypes&&... bind_args) {
  static_assert(std::is_same_v<ReturnType, void>,
                "Cannot bind function with return value to base::WeakPtr");
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  // This is a workaround for a Clang bug. If |bind_arg_cnt > func_arg_cnt|,
  // then even though the above assert will fail, Clang will still process the
  // below typedefs which might instantiate std::index_sequence with huge count
  // which can crash the compiler.
  constexpr size_t remaining_arg_count =
      func_arg_cnt - std::min(bind_arg_cnt, func_arg_cnt);

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, remaining_arg_count,
                          FunctionArgumentTypes...>;
  using ResultType =
      detail::BindResultType<OnceCallback, ReturnType, RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return detail::BindAccessHelper::Create<ResultType>(
      std::make_unique<detail::MemberFunctionWeakPtrOnceCallback<
          Class, ReturnType, BoundArgumentsType, RemainingArgumentsType>>(
          member_function_ptr, std::move(object), std::move(bounded_args)));
}

template <typename LambdaType,
          typename... BindArgumentTypes,
          typename = std::enable_if_t<traits::IsCapturelessLambdaV<LambdaType>>>
auto BindOnce(LambdaType&& lambda_type, BindArgumentTypes&&... args) {
  return BindOnce(+std::forward<LambdaType>(lambda_type),
                  std::forward<BindArgumentTypes>(args)...);
}

template <typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindOnce(OnceCallback<ReturnType(FunctionArgumentTypes...)>&& callback,
              BindArgumentTypes&&... bind_args) {
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  // This is a workaround for a Clang bug. If |bind_arg_cnt > func_arg_cnt|,
  // then even though the above assert will fail, Clang will still process the
  // below typedefs which might instantiate std::index_sequence with huge count
  // which can crash the compiler.
  constexpr size_t remaining_arg_count =
      func_arg_cnt - std::min(bind_arg_cnt, func_arg_cnt);

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, remaining_arg_count,
                          FunctionArgumentTypes...>;
  using ResultType =
      detail::BindResultType<OnceCallback, ReturnType, RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return detail::BindAccessHelper::Create<ResultType>(
      std::make_unique<detail::BoundCallbackOnceCallback<
          ReturnType, BoundArgumentsType, RemainingArgumentsType>>(
          detail::BindAccessHelper::ExtractImpl(std::move(callback)),
          std::move(bounded_args)));
}

template <typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindOnce(
    const RepeatingCallback<ReturnType(FunctionArgumentTypes...)>& callback,
    BindArgumentTypes&&... bind_args) {
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  // This is a workaround for a Clang bug. If |bind_arg_cnt > func_arg_cnt|,
  // then even though the above assert will fail, Clang will still process the
  // below typedefs which might instantiate std::index_sequence with huge count
  // which can crash the compiler.
  constexpr size_t remaining_arg_count =
      func_arg_cnt - std::min(bind_arg_cnt, func_arg_cnt);

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, remaining_arg_count,
                          FunctionArgumentTypes...>;
  using ResultType =
      detail::BindResultType<OnceCallback, ReturnType, RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return detail::BindAccessHelper::Create<ResultType>(
      std::make_unique<detail::BoundCallbackOnceCallback<
          ReturnType, BoundArgumentsType, RemainingArgumentsType>>(
          detail::BindAccessHelper::CloneImpl(callback),
          std::move(bounded_args)));
}

//
// BindRepeating()
//

template <typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindRepeating(ReturnType (*function_ptr)(FunctionArgumentTypes...),
                   BindArgumentTypes&&... bind_args) {
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  // This is a workaround for a Clang bug. If |bind_arg_cnt > func_arg_cnt|,
  // then even though the above assert will fail, Clang will still process the
  // below typedefs which might instantiate std::index_sequence with huge count
  // which can crash the compiler.
  constexpr size_t remaining_arg_count =
      func_arg_cnt - std::min(bind_arg_cnt, func_arg_cnt);

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, remaining_arg_count,
                          FunctionArgumentTypes...>;
  using ResultType = detail::BindResultType<RepeatingCallback, ReturnType,
                                            RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return detail::BindAccessHelper::Create<ResultType>(
      std::make_unique<detail::FreeFunctionRepeatingCallback<
          ReturnType, BoundArgumentsType, RemainingArgumentsType>>(
          function_ptr, std::move(bounded_args)));
}

template <typename Class,
          typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindRepeating(
    ReturnType (Class::*member_function_ptr)(FunctionArgumentTypes...),
    Class* object,
    BindArgumentTypes&&... bind_args) {
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  // This is a workaround for a Clang bug. If |bind_arg_cnt > func_arg_cnt|,
  // then even though the above assert will fail, Clang will still process the
  // below typedefs which might instantiate std::index_sequence with huge count
  // which can crash the compiler.
  constexpr size_t remaining_arg_count =
      func_arg_cnt - std::min(bind_arg_cnt, func_arg_cnt);

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, remaining_arg_count,
                          FunctionArgumentTypes...>;
  using ResultType = detail::BindResultType<RepeatingCallback, ReturnType,
                                            RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return detail::BindAccessHelper::Create<ResultType>(
      std::make_unique<detail::MemberFunctionRepeatingCallback<
          Class, ReturnType, BoundArgumentsType, RemainingArgumentsType>>(
          member_function_ptr, object, std::move(bounded_args)));
}

template <typename Class,
          typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindRepeating(
    ReturnType (Class::*member_function_ptr)(FunctionArgumentTypes...),
    WeakPtr<Class> object,
    BindArgumentTypes&&... bind_args) {
  static_assert(std::is_same_v<ReturnType, void>,
                "Cannot bind function with return value to base::WeakPtr");
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  // This is a workaround for a Clang bug. If |bind_arg_cnt > func_arg_cnt|,
  // then even though the above assert will fail, Clang will still process the
  // below typedefs which might instantiate std::index_sequence with huge count
  // which can crash the compiler.
  constexpr size_t remaining_arg_count =
      func_arg_cnt - std::min(bind_arg_cnt, func_arg_cnt);

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, remaining_arg_count,
                          FunctionArgumentTypes...>;
  using ResultType = detail::BindResultType<RepeatingCallback, ReturnType,
                                            RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return detail::BindAccessHelper::Create<ResultType>(
      std::make_unique<detail::MemberFunctionWeakPtrRepeatingCallback<
          Class, ReturnType, BoundArgumentsType, RemainingArgumentsType>>(
          member_function_ptr, std::move(object), std::move(bounded_args)));
}

template <typename LambdaType,
          typename... BindArgumentTypes,
          typename = std::enable_if_t<traits::IsCapturelessLambdaV<LambdaType>>>
auto BindRepeating(LambdaType&& lambda_type, BindArgumentTypes&&... args) {
  return BindRepeating(+std::forward<LambdaType>(lambda_type),
                       std::forward<BindArgumentTypes>(args)...);
}

template <typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindRepeating(
    const RepeatingCallback<ReturnType(FunctionArgumentTypes...)>& callback,
    BindArgumentTypes&&... bind_args) {
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  // This is a workaround for a Clang bug. If |bind_arg_cnt > func_arg_cnt|,
  // then even though the above assert will fail, Clang will still process the
  // below typedefs which might instantiate std::index_sequence with huge count
  // which can crash the compiler.
  constexpr size_t remaining_arg_count =
      func_arg_cnt - std::min(bind_arg_cnt, func_arg_cnt);

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, remaining_arg_count,
                          FunctionArgumentTypes...>;
  using ResultType = detail::BindResultType<RepeatingCallback, ReturnType,
                                            RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return detail::BindAccessHelper::Create<ResultType>(
      std::make_unique<detail::BoundCallbackRepeatingCallback<
          ReturnType, BoundArgumentsType, RemainingArgumentsType>>(
          detail::BindAccessHelper::CloneImpl(callback),
          std::move(bounded_args)));
}

}  // namespace base
