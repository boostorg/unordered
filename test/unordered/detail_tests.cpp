// Copyright 2022 Christian Mazakas.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off
#include "../helpers/prefix.hpp"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "../helpers/postfix.hpp"
// clang-format on

#include "../helpers/test.hpp"

UNORDERED_AUTO_TEST (modulo_tests) {
  std::size_t* sizes = boost::unordered::detail::prime_fmod_size<>::sizes;

  std::size_t const sizes_len =
    boost::unordered::detail::prime_fmod_size<>::sizes_len;

  // simplification of test, only use the lower 32 bits so we avoid the mixing
  // effects and get consistent results
  //
  std::size_t const hash = 1337u;
  for (std::size_t i = 0; i < sizes_len; ++i) {
    BOOST_TEST_EQ(
      boost::unordered::detail::prime_fmod_size<>::position(hash, i),
      hash % sizes[i]);
  }
}

RUN_TESTS()
