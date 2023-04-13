/* Copyright 2023 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_FOA_TUPLE_ROTATE_RIGHT_HPP
#define BOOST_UNORDERED_DETAIL_FOA_TUPLE_ROTATE_RIGHT_HPP

#include <boost/mp11/integer_sequence.hpp>
#include <tuple>
#include <utility>

namespace boost{
namespace unordered{
namespace detail{
namespace foa{

template<std::size_t... Is,typename Tuple>
auto tuple_rotate_right_aux(mp11::index_sequence<Is...>,Tuple&& x)
  ->std::tuple<decltype(
    std::get<(Is+sizeof...(Is)-1)%sizeof...(Is)>(std::forward<Tuple>(x)))...
  >
{
  return std::tuple<decltype(
    std::get<(Is+sizeof...(Is)-1)%sizeof...(Is)>(std::forward<Tuple>(x)))...
  >
  {
    std::get<(Is+sizeof...(Is)-1)%sizeof...(Is)>(std::forward<Tuple>(x))...
  };
}

template<typename Tuple>
auto tuple_rotate_right(Tuple&& x)
  ->decltype(tuple_rotate_right_aux(
      mp11::make_index_sequence<std::tuple_size<Tuple>::value>{},
      std::forward<Tuple>(x)))
{
  return tuple_rotate_right_aux(
    mp11::make_index_sequence<std::tuple_size<Tuple>::value>{},
    std::forward<Tuple>(x));
}

} /* namespace foa */
} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#endif
