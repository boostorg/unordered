/* Copyright 2023 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_FOA_TAKES_ARG_AS_CONST_REFERENCE_HPP
#define BOOST_UNORDERED_DETAIL_FOA_TAKES_ARG_AS_CONST_REFERENCE_HPP

#include <boost/type_traits/make_void.hpp>
#include <type_traits>
#include <utility>

namespace boost{
namespace unordered{
namespace detail{
namespace foa{

template<typename Arg,typename R,typename... Args>
void check_function_1st_arg(R(*)(Arg,Args...));

template<typename Arg,typename R,typename... Args>
void check_function_1st_arg(R(*)(Arg,Args...,...));

#define BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(qualifier)       \
template<typename Arg,typename R,typename C,typename... Args> \
void check_memfun_1st_arg(R(C::*)(Arg,Args...)qualifier);     \
                                                              \
template<typename Arg,typename R,typename C,typename... Args> \
void check_memfun_1st_arg(R(C::*)(Arg,Args...,...)qualifier);

/* VS warns when a pp function is directly called with an empty arg */
#define BOOST_UNORDERED_EMPTY_PP_ARG()

BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(BOOST_UNORDERED_EMPTY_PP_ARG())
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(const)
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(volatile)
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(const volatile)
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(&)
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(const&)
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(volatile&)
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(const volatile&)
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(&&)
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(const&&)
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(volatile&&)
BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG(const volatile&&)

#undef BOOST_UNORDERED_EMPTY_PP_ARG
#undef BOOST_UNORDERED_CHECK_MEMFUN_1ST_ARG

/* Detects if f(x) takes x as a const reference. From an implementation
 * technique by Kenneth Gorking.
 * Requires: F is invocable with an Arg&.
 */

template<typename F,typename Arg,typename=void>
struct takes_arg_as_const_reference0:std::false_type{};

template<typename F,typename Arg>
struct takes_arg_as_const_reference0<
  F,Arg,
  decltype(check_function_1st_arg<const Arg&>(std::declval<F>()))
>:std::true_type{};

template<typename F,typename Arg>
struct takes_arg_as_const_reference0<
  F,Arg,
  boost::void_t<
    decltype(std::declval<F>().operator()(std::declval<Arg&>()))
  >
>:std::true_type{};

template<typename F,typename Arg,typename=void>
struct takes_arg_as_const_reference:takes_arg_as_const_reference0<F,Arg>{};

template<
  typename F,typename Arg,
  typename R=decltype(std::declval<F>()(std::declval<Arg&>())),
  typename RawF=
    typename std::remove_cv<typename std::remove_reference<F>::type>::type
>
decltype(check_memfun_1st_arg<Arg&,R,RawF>(&RawF::operator()))
check_operator_call_takes_arg_as_reference();

template<typename F,typename Arg>
struct takes_arg_as_const_reference<
  F,Arg,
  decltype(check_operator_call_takes_arg_as_reference<F,Arg>())
>:std::false_type{};

} /* namespace foa */
} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#endif
