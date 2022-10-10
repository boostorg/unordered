// Copyright (C) 2022 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNORDERED_DETAIL_TYPE_TRAITS_HPP
#define BOOST_UNORDERED_DETAIL_TYPE_TRAITS_HPP

#include <boost/config.hpp>
#if defined(BOOST_HAS_PRAGMA_ONCE)
#pragma once
#endif

#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/make_void.hpp>

namespace boost {
  namespace unordered {
    namespace detail {
      ////////////////////////////////////////////////////////////////////////////
      // Type checkers used for the transparent member functions added by C++20
      // and up

      template <class, class = void>
      struct is_transparent : public boost::false_type
      {
      };

      template <class T>
      struct is_transparent<T,
        typename boost::make_void<typename T::is_transparent>::type>
          : public boost::true_type
      {
      };

      template <class, class A, class B> struct are_transparent
      {
        static bool const value =
          is_transparent<A>::value && is_transparent<B>::value;
      };

      template <class Key, class UnorderedMap> struct transparent_non_iterable
      {
        typedef typename UnorderedMap::hasher hash;
        typedef typename UnorderedMap::key_equal key_equal;
        typedef typename UnorderedMap::iterator iterator;
        typedef typename UnorderedMap::const_iterator const_iterator;

        static bool const value =
          are_transparent<Key, hash, key_equal>::value &&
          !boost::is_convertible<Key, iterator>::value &&
          !boost::is_convertible<Key, const_iterator>::value;
      };
    } // namespace detail
  }   // namespace unordered
} // namespace boost

#endif // BOOST_UNORDERED_DETAIL_TYPE_TRAITS_HPP
