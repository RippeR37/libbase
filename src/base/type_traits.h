#pragma once

#include <tuple>
#include <utility>

namespace base {
namespace traits {

/*
 * Identity<T>
 *
 *   ::type - will hold T
 *
 * Helper, used sometimes to disable template argument deduction on specific
 * arguments.
 */
template <typename T>
struct IdentityType {
  using type = T;
};

template <typename T>
using IdentityT = typename IdentityType<T>::type;

/*
 * RemoveCVRef<T>
 *
 *   ::type - holds |T| with removed c/v qualifiers and l/r-value reference.
 */
template <typename T>
struct RemoveCVRef {
  using type = std::remove_reference_t<std::remove_cv_t<T>>;
};
template <typename T>
using RemoveCVRefT = typename RemoveCVRef<T>::type;

/*
 * TypesRange<Offset, std::tuple<T0, T1, ..., TN>, IndexSequence<M>
 *
 *   ::type - will hold std::tuple<T0, T1, ..., TM>  (M <= N)
 */
template <size_t Offset, typename TupleType, typename IndexSequence>
struct TypesRange {};

template <size_t Offset, typename TupleType, size_t... Indexes>
struct TypesRange<Offset, TupleType, std::index_sequence<Indexes...>> {
  static_assert(sizeof...(Indexes) <= std::tuple_size_v<TupleType>,
                "Cannot take more types then provided");
  using type = std::tuple<
      typename std::tuple_element<Indexes + Offset, TupleType>::type...>;
};

/*
 * Convienience aliases for taking arbitraty sub-range of given size of provided
 * types.
 *
 * TypesRangeT<Offset, Count, std::tuple<T[0], ..., T[N]>> =
 *   std::tuple<T[Offset], ..., T[Offset+Count-1]>
 */
template <size_t Offset, size_t Count, typename TupleType>
using TypesRangeT = typename TypesRange<Offset,
                                        TupleType,
                                        std::make_index_sequence<Count>>::type;

/*
 * Convienience aliase for taking arbitraty prefix of given size of provided
 * types.
 *
 * HeadTypesRangeT<Count, std::tuple<T[0], ..., T[N]>> =
 *   std::tuple<T[0], ..., T[Count-1]>
 */
template <size_t Count, typename TupleType>
using HeadTypesRangeT = TypesRangeT<0, Count, TupleType>;

//
// IsFunctionPointer<T>
//
template <typename T>
struct IsFunctionPointer : std::false_type {};

template <typename T, typename... Args>
struct IsFunctionPointer<T (*)(Args...)> : std::true_type {};

template <typename T>
inline constexpr bool IsFunctionPointerV = IsFunctionPointer<T>::value;

//
// IsCapturelessLambdaV<T>
//
template <typename T, typename = decltype(+std::declval<T>())>
inline constexpr bool IsCapturelessLambdaSfinaeV =
    !std::is_pointer_v<T> && IsFunctionPointerV<decltype(+std::declval<T>())>;

template <typename Lambda, typename = void>
struct IsCapturelessLambda {
  static constexpr bool value = false;
};

template <typename Lambda>
struct IsCapturelessLambda<
    Lambda,
    std::enable_if_t<traits::IsCapturelessLambdaSfinaeV<Lambda>>> {
  static constexpr bool value = true;
};

template <typename Lambda>
inline constexpr bool IsCapturelessLambdaV = IsCapturelessLambda<Lambda>::value;

}  // namespace traits
}  // namespace base
