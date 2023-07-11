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

static constexpr bool noexcept_is_part_of_signature=
  !std::is_same<void(*)(),void(*)()noexcept>::value;

template<typename Arg,typename Sig>
static std::false_type has_1st_arg(Sig);

template<typename Arg,typename R,typename... Args>
static std::true_type has_1st_arg(R(*)(Arg,Args...));

template<
  typename Arg,typename R,typename... Args,
  bool dependent_value=false,
  typename std::enable_if<
    noexcept_is_part_of_signature||dependent_value>::type* =nullptr
>
static std::true_type has_1st_arg(R(*)(Arg,Args...)noexcept);

template<typename Arg,typename R,typename... Args>
static std::true_type has_1st_arg(R(*)(Arg,Args...,...));

template<
  typename Arg,typename R,typename... Args,
  bool dependent_value=false,
  typename std::enable_if<
    noexcept_is_part_of_signature||dependent_value>::type* =nullptr
>
std::true_type has_1st_arg(R(*)(Arg,Args...,...)noexcept);

#define BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(qualifier)             \
template<typename Arg,typename R,typename C,typename... Args>                 \
static std::true_type has_1st_arg(R(C::*)(Arg,Args...)qualifier);             \
                                                                              \
template<                                                                     \
  typename Arg,typename R,typename C,typename... Args,                        \
  bool dependent_value=false,                                                 \
  typename std::enable_if<                                                    \
    noexcept_is_part_of_signature||dependent_value>::type* =nullptr           \
>                                                                             \
static std::true_type has_1st_arg(R(C::*)(Arg,Args...)qualifier noexcept);    \
                                                                              \
template<typename Arg,typename R,typename C,typename... Args>                 \
static std::true_type has_1st_arg(R(C::*)(Arg,Args...,...)qualifier);         \
                                                                              \
template<                                                                     \
  typename Arg,typename R,typename C,typename... Args,                        \
  bool dependent_value=false,                                                 \
  typename std::enable_if<                                                    \
    noexcept_is_part_of_signature||dependent_value>::type* =nullptr           \
>                                                                             \
static std::true_type has_1st_arg(R(C::*)(Arg,Args...,...)qualifier noexcept);

/* VS warns when a pp function is directly called with an empty arg */
#define BOOST_UNORDERED_EMPTY_PP_ARG()

BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(BOOST_UNORDERED_EMPTY_PP_ARG())
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(const)
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(volatile)
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(const volatile)
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(&)
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(const&)
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(volatile&)
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(const volatile&)
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(&&)
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(const&&)
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(volatile&&)
BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(const volatile&&)

#undef BOOST_UNORDERED_EMPTY_PP_ARG
#undef BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN

/* Detects if f(x) takes x as a const reference. From an implementation
 * technique by Kenneth Gorking.
 * Requires: F is invocable with an Arg&.
 */

template<typename F,typename Arg,typename=void>
struct takes_arg_as_const_reference0:
decltype(has_1st_arg<const Arg&>(std::declval<F>())){};

template<typename F,typename Arg>
struct takes_arg_as_const_reference0<
  F,Arg,
  boost::void_t<
    decltype(has_1st_arg<const Arg&>(&F::template operator()<Arg>))
  >
>:
decltype(has_1st_arg<const Arg&>(&F::template operator()<Arg>)){};

template<typename F,typename Arg,typename=void>
struct takes_arg_as_const_reference:takes_arg_as_const_reference0<F,Arg>{};

template<typename F,typename Arg>
struct takes_arg_as_const_reference<
  F,Arg,
  boost::void_t<
    decltype(has_1st_arg<const Arg&>(&F::operator()))
  >
>:
decltype(has_1st_arg<const Arg&>(&F::operator())){};

} /* namespace foa */
} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#endif
