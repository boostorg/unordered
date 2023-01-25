
// Copyright 2006-2009 Daniel James.
// Copyright 2022-2023 Christian Mazakas.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// This test checks the runtime requirements of containers.

#include "../helpers/unordered.hpp"

#include "../helpers/equivalent.hpp"
#include "../helpers/generators.hpp"
#include "../helpers/test.hpp"
#include <algorithm>
#include <cstdlib>

test::seed_t initialize_seed(14878);

template <class X> void simple_test(X const& a)
{
  test::unordered_equivalence_tester<X> equivalent(a);

  {
    X u;
    BOOST_TEST(u.size() == 0);
    BOOST_TEST(X().size() == 0);
  }

  {
    BOOST_TEST(equivalent(X(a)));
  }

  {
    X u(a);
    BOOST_TEST(equivalent(u));
  }

  {
    X u = a;
    BOOST_TEST(equivalent(u));
  }

  {
    X b(a);
    BOOST_TEST(b.begin() == const_cast<X const&>(b).cbegin());
    BOOST_TEST(b.end() == const_cast<X const&>(b).cend());
  }

  {
    X b(a);
    X c;
    BOOST_TEST(equivalent(b));
    BOOST_TEST(c.empty());
    b.swap(c);
    BOOST_TEST(b.empty());
    BOOST_TEST(equivalent(c));
    b.swap(c);
    BOOST_TEST(c.empty());
    BOOST_TEST(equivalent(b));
  }

  {
    X u;
    X& r = u;
    BOOST_TEST(&(r = r) == &r);

    BOOST_TEST(r.empty());
    BOOST_TEST(&(r = a) == &r);
    BOOST_TEST(equivalent(r));
    BOOST_TEST(&(r = r) == &r);
    BOOST_TEST(equivalent(r));
  }

  {
    BOOST_TEST(a.size() == static_cast<typename X::size_type>(
                             std::distance(a.begin(), a.end())));
  }

  {
    BOOST_TEST(a.empty() == (a.size() == 0));
  }

  {
    BOOST_TEST(a.empty() == (a.begin() == a.end()));
    X u;
    BOOST_TEST(u.begin() == u.end());
  }
}

template <class X> static void simple_set_tests(X*)
{
  X set;

  simple_test(set);

  set.insert(1);
  set.insert(2);
  set.insert(1456);
  simple_test(set);
}

#ifndef BOOST_UNORDERED_FOA_TESTS
template <class X> static void simple_multiset_tests(X*)
{
  X multiset;
  simple_test(multiset);

  for (int i1 = 0; i1 < 1000; ++i1) {
    int count = rand() % 10, index = rand();
    for (int j = 0; j < count; ++j)
      multiset.insert(index);
  }
  simple_test(multiset);
}
#endif

template <class X> static void simple_map_tests(X*)
{
  X map;

  for (int i2 = 0; i2 < 1000; ++i2) {
    map.insert(std::pair<const int, int>(rand(), rand()));
  }
  simple_test(map);
}

#ifndef BOOST_UNORDERED_FOA_TESTS
template <class X> static void simple_multimap_tests(X*)
{
  X multimap;

  for (int i3 = 0; i3 < 1000; ++i3) {
    int count = rand() % 10, index = rand();
    for (int j = 0; j < count; ++j)
      multimap.insert(std::pair<const int, int>(index, rand()));
  }
  simple_test(multimap);
}
#endif

#ifdef BOOST_UNORDERED_FOA_TESTS
static boost::unordered_flat_set<int>* flat_set;
static boost::unordered_flat_map<int, int>* flat_map;
static boost::unordered_node_set<int>* node_set;
static boost::unordered_node_map<int, int>* node_map;

UNORDERED_TEST(simple_map_tests, ((flat_map)(node_map)))
UNORDERED_TEST(simple_set_tests, ((flat_set)(node_set)))
#else
static boost::unordered_set<int>* set;
static boost::unordered_map<int, int>* map;
static boost::unordered_multiset<int>* multiset;
static boost::unordered_multimap<int, int>* multimap;

UNORDERED_TEST(simple_set_tests, ((set)))
UNORDERED_TEST(simple_map_tests, ((map)))
UNORDERED_TEST(simple_multiset_tests, ((multiset)))
UNORDERED_TEST(simple_multimap_tests, ((multimap)))

#endif

RUN_TESTS()
