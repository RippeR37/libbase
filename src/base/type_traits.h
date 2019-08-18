#pragma once

#include <tuple>
#include <utility>

#include "callback.h"

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
 * Convienience aliases for taking arbitraty sub-range, prefix or suffix of
 * given size of provided types.
 *
 * TypesRangeT<Offset, Count, T[0], ..., T[N]> =
 *   std::tuple<T[Offset], ..., T[Offset+Count-1]>
 *
 * HeadTypesRangeT<Count, T[0], ..., T[N]> = std::tuple<T[0], ..., T[Count-1]>
 */
template <size_t Offset, size_t Count, typename... Types>
using TypesRangeT = typename TypesRange<Offset,
                                        std::tuple<Types...>,
                                        std::make_index_sequence<Count>>::type;

template <size_t Count, typename... Types>
using HeadTypesRangeT = TypesRangeT<0, Count, Types...>;

}  // namespace traits
}  // namespace base
