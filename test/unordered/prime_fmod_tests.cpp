// Copyright 2022 Christian Mazakas.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off
#include "../helpers/prefix.hpp"
#include <boost/unordered/detail/fca.hpp>
#include "../helpers/postfix.hpp"
// clang-format on

#include <boost/core/lightweight_test.hpp>
#include <boost/static_assert.hpp>

#if !defined(SIZE_MAX) || !defined(UINTPTR_MAX)
BOOST_STATIC_ASSERT_MSG(false, "missing required macros");
#endif

#if defined(SIZE_MAX)
#if ((((SIZE_MAX >> 16) >> 16) >> 16) >> 15) != 0

#if !defined(BOOST_UNORDERED_FCA_HAS_64B_SIZE_T)
BOOST_STATIC_ASSERT_MSG(false, "missing required macros");
#endif

#endif /* ((((SIZE_MAX >> 16) >> 16) >> 16) >> 15) != 0 */

#elif defined(UINTPTR_MAX) /* defined(SIZE_MAX) is false */

#if ((((UINTPTR_MAX >> 16) >> 16) >> 16) >> 15) != 0

#if !defined(BOOST_UNORDERED_FCA_HAS_64B_SIZE_T)
BOOST_STATIC_ASSERT_MSG(false, "missing required macros");
#endif

#endif /* ((((UINTPTR_MAX >> 16) >> 16) >> 16) >> 15) != 0 */
#endif /* defined(UINTPTR_MAX) */

// Pretty inefficient, but the test is fast enough.
// Might be too slow if we had larger primes?
bool is_prime(std::size_t x)
{
  if (x == 2) {
    return true;
  } else if (x == 1 || x % 2 == 0) {
    return false;
  } else {
    // y*y <= x had rounding errors, so instead use y <= (x/y).
    for (std::size_t y = 3; y <= (x / y); y += 2) {
      if (x % y == 0) {
        return false;
        break;
      }
    }

    return true;
  }
}

void prime_sizes_test()
{
  // just some basic sanity checks
  //
  BOOST_TEST(!is_prime(0));
  BOOST_TEST(!is_prime(1));
  BOOST_TEST(is_prime(2));
  BOOST_TEST(is_prime(3));
  BOOST_TEST(is_prime(13));
  BOOST_TEST(!is_prime(4));
  BOOST_TEST(!is_prime(100));

  std::size_t* sizes = boost::unordered::detail::prime_fmod_size<>::sizes;
  std::size_t sizes_len =
    boost::unordered::detail::prime_fmod_size<>::sizes_len;

  // prove every number in our sizes array is prime
  //
  BOOST_TEST_GT(sizes_len, 0);

  for (std::size_t i = 0; i < sizes_len; ++i) {
    BOOST_TEST(is_prime(sizes[i]));
  }

  // prove that every subsequent number in the sequence is larger than the
  // previous
  //
  for (std::size_t i = 1; i < sizes_len; ++i) {
    BOOST_TEST_GT(sizes[i], sizes[i - 1]);
  }

#if defined(BOOST_UNORDERED_FCA_FASTMOD_SUPPORT)
  // now we wish to prove that if we do have the reciprocals stored, we have the
  // correct amount of them, i.e. one for every entry in sizes[] that fits in 32
  // bits
  //
  std::size_t* inv_sizes32 =
    boost::unordered::detail::prime_fmod_size<>::inv_sizes32;

  std::size_t inv_sizes32_len =
    boost::unordered::detail::prime_fmod_size<>::inv_sizes32_len;

  std::size_t count = 0;
  for (std::size_t i = 0; i < sizes_len; ++i) {
    if (sizes[i] <= UINT32_MAX) {
      ++count;
    }
  }

  BOOST_TEST_GT(inv_sizes32_len, 0);
  BOOST_TEST_EQ(inv_sizes32_len, count);

  // these values should also be monotonically decreasing
  //
  for (std::size_t i = 1; i < inv_sizes32_len; ++i) {
    BOOST_TEST_LT(inv_sizes32[i], inv_sizes32[i - 1]);
  }
#endif
}

void modulo_tests()
{
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

int main()
{
  prime_sizes_test();
  modulo_tests();

  return boost::report_errors();
}
