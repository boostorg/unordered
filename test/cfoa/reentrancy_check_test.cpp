// Copyright 2023 Joaquin M Lopez Munoz
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <cstdlib>

#define BOOST_ENABLE_ASSERT_HANDLER 

static bool reentrancy_detected = false;

namespace boost {
  // Caveat lector: a proper handler shouldn't throw as it may be executed
  // within a noexcept function.

void assertion_failed_msg(
  char const*, char const*, char const*, char const*, long)
{
  reentrancy_detected = true;
  throw 0;
}

void assertion_failed(char const*, char const*, char const*, long) // LCOV_EXCL_START
{
  std::abort();
}                                                                  // LCOV_EXCL_STOP

}

#include <boost/unordered/concurrent_flat_map.hpp>
#include <boost/core/lightweight_test.hpp>

template<typename F>
void detect_reentrancy(F f)
{
  reentrancy_detected = false;
  try {
    f();
  }
  catch(int) {}
  BOOST_TEST(reentrancy_detected);
}

int main()
{
  using map = boost::concurrent_flat_map<int, int>;
  using value_type = typename map::value_type;

  map m1, m2;
  m1.emplace(0, 0);
  m2.emplace(1, 0);

  detect_reentrancy([&] {
    m1.visit_all([&](value_type&) { (void)m1.contains(0); });
  }); // LCOV_EXCL_LINE

  detect_reentrancy([&] {
    m1.visit_all([&](value_type&) { m1.rehash(0); });
  }); // LCOV_EXCL_LINE

  detect_reentrancy([&] {
    m1.visit_all([&](value_type&) { 
      m2.visit_all([&](value_type&) { 
        m1=m2;
      }); // LCOV_EXCL_START
    });
  });     // LCOV_EXCL_STOP

  detect_reentrancy([&] {
    m1.visit_all([&](value_type&) { 
      m2.visit_all([&](value_type&) { 
        m2=m1;
      }); // LCOV_EXCL_START
    });
  });     // LCOV_EXCL_STOP

  return boost::report_errors();
}
