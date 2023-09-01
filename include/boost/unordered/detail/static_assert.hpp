#ifndef BOOST_UNORDERED_DETAIL_STATIC_ASSERT_HPP
#define BOOST_UNORDERED_DETAIL_STATIC_ASSERT_HPP

#include <boost/config.hpp>
#if defined(BOOST_HAS_PRAGMA_ONCE)
#pragma once
#endif

#define BOOST_UNORDERED_STATIC_ASSERT(...)                                     \
  static_assert(__VA_ARGS__, #__VA_ARGS__)

#endif // BOOST_UNORDERED_DETAIL_STATIC_ASSERT_HPP