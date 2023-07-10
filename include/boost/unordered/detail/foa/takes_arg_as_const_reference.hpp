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

/* VS warns when a pp function is directly called with an empty arg */
#define BOOST_UNORDERED_EMPTY_PP_ARG()

namespace boost{
namespace unordered{
namespace detail{
namespace foa{

template<typename Sig>
struct remove_noexcept{using type=Sig;};

template<typename Sig>
using remove_noexcept_t=typename remove_noexcept<Sig>::type;

template<typename R,typename... Args>
struct remove_noexcept<R(Args...)noexcept>
  {using type=R(Args...);};

template<typename R,typename... Args>
struct remove_noexcept<R(Args...,...)noexcept>
  {using type=R(Args...,...);};

template<typename R,typename... Args>
struct remove_noexcept<R(&)(Args...)noexcept>
  {using type=R(&)(Args...);};

template<typename R,typename... Args>
struct remove_noexcept<R(&)(Args...,...)noexcept>
  {using type=R(&)(Args...,...);};

template<typename R,typename... Args>
struct remove_noexcept<R(*)(Args...)noexcept>
  {using type=R(Args...);};

template<typename R,typename... Args>
struct remove_noexcept<R(*)(Args...,...)noexcept>
  {using type=R(Args...,...);};

#define BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(qualifier)      \
template<typename R,typename C,typename... Args>               \
struct remove_noexcept<R(C::*)(Args...)qualifier noexcept>     \
  {using type=R(C::*)(Args...)qualifier;};                     \
                                                               \
template<typename R,typename C,typename... Args>               \
struct remove_noexcept<R(C::*)(Args...,...)qualifier noexcept> \
  {using type=R(C::*)(Args...,...)qualifier;};

BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(BOOST_UNORDERED_EMPTY_PP_ARG())
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(const)
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(volatile)
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(const volatile)
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(&)
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(const&)
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(volatile&)
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(const volatile&)
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(&&)
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(const&&)
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(volatile&&)
BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN(const volatile&&)

#undef BOOST_UNORDERED_REMOVE_NOEXCEPT_MEMFUN

template<typename Sig>
struct has_const_reference_arg:std::false_type{};

template<typename R,typename Arg,typename... Args>
struct has_const_reference_arg<
  R(const Arg&,Args...)
>:std::true_type{};

template<typename R,typename Arg,typename... Args>
struct has_const_reference_arg<
  R(const Arg&,Args...,...)
>:std::true_type{};

template<typename R,typename Arg,typename... Args>
struct has_const_reference_arg<
  R(&)(const Arg&,Args...)
>:std::true_type{};

template<typename R,typename Arg,typename... Args>
struct has_const_reference_arg<
  R(&)(const Arg&,Args...,...)
>:std::true_type{};

template<typename R,typename Arg,typename... Args>
struct has_const_reference_arg<
  R(*)(const Arg&,Args...)  
>:std::true_type{};

template<typename R,typename Arg,typename... Args>
struct has_const_reference_arg<
  R(*)(const Arg&,Args...,...)
>:std::true_type{};

#define BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN(qualifier) \
template<typename R,typename C,typename Arg,typename... Args>     \
struct has_const_reference_arg<                                   \
  R(C::*)(const Arg&,Args...)qualifier                            \
>:std::true_type{};                                               \
                                                                  \
template<typename R,typename C,typename Arg,typename... Args>     \
struct has_const_reference_arg<                                   \
  R(C::*)(const Arg&,Args...,...)qualifier                        \
>:std::true_type{};

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

#undef BOOST_UNORDERED_HAS_CONST_REFERENCE_ARG_MEMFUN

/* Detects if f(x) takes x as a const reference. From an implementation
 * technique by Kenneth Gorking.
 * Requires: F is invocable with an Arg&.
 */

template<typename F,typename /*Arg*/,typename=void>
struct takes_arg_as_const_reference:
  has_const_reference_arg<remove_noexcept_t<F>>{};

/* VS2017 and older issue a C3517 error when trying to obtain the type of
 * an instantiation of a function template with deduced return type if
 * the instantiation has not been evaluated before. Passing through this
 * function solves the problem.
 */
template<typename T> T force_evaluation(T);

template<typename F,typename Arg>
struct takes_arg_as_const_reference<
  F,Arg,
  boost::void_t<decltype(force_evaluation(&F::template operator()<Arg>))>
>:
takes_arg_as_const_reference<
  decltype(force_evaluation(&F::template operator()<Arg>)),Arg
>{};

template<typename F,typename Arg>
struct takes_arg_as_const_reference<
  F,Arg,
  boost::void_t<decltype(&F::operator())>
>:
takes_arg_as_const_reference<
  decltype(&F::operator()),Arg
>{};


} /* namespace foa */
} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#undef BOOST_UNORDERED_EMPTY_PP_ARG

#endif
