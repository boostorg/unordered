// Copyright 2023 Joaquin M Lopez Munoz
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#define BOOST_UNORDERED_ENABLE_REENTRANCY_CHECK_HANDLER

static bool reentrancy_detected = false;

namespace boost {
  // Caveat lector: a proper handler should terminate as it may be executed
  // within a noexcept function.

  void boost_unordered_reentrancy_check_failed()
  {
    reentrancy_detected = true;
    throw 0;
  }
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
  });

  detect_reentrancy([&] {
    m1.visit_all([&](value_type&) { m1.rehash(0); });
  });

  detect_reentrancy([&] {
    m1.visit_all([&](value_type&) { 
      m2.visit_all([&](value_type&) { 
        m1=m2;
      });
    });
  });

  detect_reentrancy([&] {
    m1.visit_all([&](value_type&) { 
      m2.visit_all([&](value_type&) { 
        m2=m1;
      });
    });
  });

  return boost::report_errors();
}
