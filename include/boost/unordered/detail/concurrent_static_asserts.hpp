/* Copyright 2023 Christian Mazakas.
 * Copyright 2023 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_CONCURRENT_STATIC_ASSERTS_HPP
#define BOOST_UNORDERED_DETAIL_CONCURRENT_STATIC_ASSERTS_HPP

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

#include <functional>
#include <type_traits>

#define BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)                             \
  static_assert(boost::unordered::detail::is_invocable<F, value_type&>::value, \
    "The provided Callable must be invocable with value_type&");

#define BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)                       \
  static_assert(                                                               \
    boost::unordered::detail::is_invocable<F, value_type const&>::value,       \
    "The provided Callable must be invocable with value_type const&");

#if BOOST_CXX_VERSION >= 202002L

#define BOOST_UNORDERED_STATIC_ASSERT_EXEC_POLICY(P)                           \
  static_assert(!std::is_base_of<std::execution::parallel_unsequenced_policy,  \
                  ExecPolicy>::value,                                          \
    "ExecPolicy must be sequenced.");                                          \
  static_assert(                                                               \
    !std::is_base_of<std::execution::unsequenced_policy, ExecPolicy>::value,   \
    "ExecPolicy must be sequenced.");

#else

#define BOOST_UNORDERED_STATIC_ASSERT_EXEC_POLICY(P)                           \
  static_assert(!std::is_base_of<std::execution::parallel_unsequenced_policy,  \
                  ExecPolicy>::value,                                          \
    "ExecPolicy must be sequenced.");
#endif

#define BOOST_UNORDERED_DETAIL_COMMA ,

#define BOOST_UNORDERED_DETAIL_LAST_ARG(Arg, Args)                             \
  mp11::mp_back<mp11::mp_list<Arg BOOST_UNORDERED_DETAIL_COMMA Args> >

#define BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_INVOCABLE(Arg, Args)            \
  BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(                                     \
    BOOST_UNORDERED_DETAIL_LAST_ARG(Arg, Args))

#define BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_CONST_INVOCABLE(Arg, Args)      \
  BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(                               \
    BOOST_UNORDERED_DETAIL_LAST_ARG(Arg, Args))

namespace boost {
  namespace unordered {
    namespace detail {
      template <class F, class... Args>
      struct is_invocable
          : std::is_constructible<std::function<void(Args...)>,
              std::reference_wrapper<typename std::remove_reference<F>::type> >
      {
      };

    } // namespace detail

  } // namespace unordered

} // namespace boost

#endif // BOOST_UNORDERED_DETAIL_CONCURRENT_STATIC_ASSERTS_HPP
