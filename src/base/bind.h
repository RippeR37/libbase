#pragma once

#include <tuple>

#include "callback.h"
#include "type_traits.h"

namespace base {

namespace detail {

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

}  // namespace detail

template <typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindOnce(ReturnType (*function_ptr)(FunctionArgumentTypes...),
              BindArgumentTypes&&... bind_args) {
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, func_arg_cnt - bind_arg_cnt,
                          FunctionArgumentTypes...>;
  using ResultType =
      detail::BindResultType<OnceCallback, ReturnType, RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return ResultType{function_ptr, std::move(bounded_args)};
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

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, func_arg_cnt - bind_arg_cnt,
                          FunctionArgumentTypes...>;
  using ResultType =
      detail::BindResultType<OnceCallback, ReturnType, RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return ResultType{member_function_ptr, object, std::move(bounded_args)};
}

template <typename LambdaType,
          typename... BindArgumentTypes,
          typename = std::enable_if_t<!std::is_pointer_v<LambdaType>>,
          typename = decltype(BindOnce(+std::declval<LambdaType>(),
                                       std::declval<BindArgumentTypes>()...))>
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

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, func_arg_cnt - bind_arg_cnt,
                          FunctionArgumentTypes...>;
  using ResultType =
      detail::BindResultType<OnceCallback, ReturnType, RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return ResultType{std::move(callback), std::move(bounded_args)};
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

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, func_arg_cnt - bind_arg_cnt,
                          FunctionArgumentTypes...>;
  using ResultType =
      detail::BindResultType<OnceCallback, ReturnType, RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return ResultType{std::move(callback), std::move(bounded_args)};
}

template <typename ReturnType,
          typename... FunctionArgumentTypes,
          typename... BindArgumentTypes>
auto BindRepeating(ReturnType (*function_ptr)(FunctionArgumentTypes...),
                   BindArgumentTypes&&... bind_args) {
  constexpr size_t func_arg_cnt = sizeof...(FunctionArgumentTypes);
  constexpr size_t bind_arg_cnt = sizeof...(BindArgumentTypes);
  static_assert(bind_arg_cnt <= func_arg_cnt,
                "Cannot bind more more arguments than the function takes");

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, func_arg_cnt - bind_arg_cnt,
                          FunctionArgumentTypes...>;
  using ResultType = detail::BindResultType<RepeatingCallback, ReturnType,
                                            RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return ResultType{function_ptr, std::move(bounded_args)};
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

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, func_arg_cnt - bind_arg_cnt,
                          FunctionArgumentTypes...>;
  using ResultType = detail::BindResultType<RepeatingCallback, ReturnType,
                                            RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return ResultType{member_function_ptr, object, std::move(bounded_args)};
}

template <
    typename LambdaType,
    typename... BindArgumentTypes,
    typename = std::enable_if_t<!std::is_pointer_v<LambdaType>>,
    typename = decltype(BindRepeating(+std::declval<LambdaType>(),
                                      std::declval<BindArgumentTypes>()...))>
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

  using BoundArgumentsType =
      traits::HeadTypesRangeT<bind_arg_cnt, FunctionArgumentTypes...>;
  using RemainingArgumentsType =
      traits::TypesRangeT<bind_arg_cnt, func_arg_cnt - bind_arg_cnt,
                          FunctionArgumentTypes...>;
  using ResultType = detail::BindResultType<RepeatingCallback, ReturnType,
                                            RemainingArgumentsType>;

  BoundArgumentsType bounded_args{
      std::forward<BindArgumentTypes>(bind_args)...};
  return ResultType{callback, std::move(bounded_args)};
}

}  // namespace base
