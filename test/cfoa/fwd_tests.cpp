// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "helpers.hpp"
#include <boost/config/workaround.hpp>
#include <boost/unordered/concurrent_flat_map_fwd.hpp>

test::seed_t initialize_seed{32304628};

using test::default_generator;
using test::limited_range;
using test::sequential;

template <class T>
void swap_call(boost::unordered::concurrent_flat_map<T, T>& x1,
  boost::unordered::concurrent_flat_map<T, T>& x2)
{
  swap(x1, x2);
}

template <class T>
bool equal_call(boost::unordered::concurrent_flat_map<T, T>& x1,
  boost::unordered::concurrent_flat_map<T, T>& x2)
{
  return x1 == x2;
}

template <class T>
bool unequal_call(boost::unordered::concurrent_flat_map<T, T>& x1,
  boost::unordered::concurrent_flat_map<T, T>& x2)
{
  return x1 != x2;
}

#include <boost/unordered/concurrent_flat_map.hpp>

using map_type = boost::unordered::concurrent_flat_map<int, int>;

#if BOOST_WORKAROUND(BOOST_CLANG_VERSION, != 30700)

UNORDERED_AUTO_TEST (fwd_swap_call) {
  map_type x1, x2;
  swap_call(x1, x2);
}

#endif

UNORDERED_AUTO_TEST (fwd_equal_call) {
  map_type x1, x2;
  BOOST_TEST(equal_call(x1, x2));
}

UNORDERED_AUTO_TEST (fwd_unequal_call) {
  map_type x1, x2;
  BOOST_TEST_NOT(unequal_call(x1, x2));
}

RUN_TESTS()
