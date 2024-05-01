// Copyright 2024 Joaquin M Lopez Muoz.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_UNORDERED_ENABLE_STATS

#ifdef BOOST_UNORDERED_CFOA_TESTS
#include <boost/unordered/concurrent_flat_map.hpp>
#include <boost/unordered/concurrent_flat_set.hpp>
#include "../cfoa/helpers.hpp"
#else
#include "../helpers/unordered.hpp"
#endif

#include "../helpers/helpers.hpp"
#include "../helpers/random_values.hpp"
#include "../helpers/test.hpp"

template <class T> struct unequal_allocator
{
  typedef T value_type;

  unequal_allocator(int n = 0): n_{n} {}
  unequal_allocator(unequal_allocator const&) = default;
  unequal_allocator(unequal_allocator&&) = default;

  template <class U>
  unequal_allocator(unequal_allocator<U> const& x): n_{x.n_} {}

  BOOST_ATTRIBUTE_NODISCARD T* allocate(std::size_t n)
  {
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, std::size_t) noexcept { ::operator delete(p); }

  bool operator==(unequal_allocator const& x) const { return n_ == x.n_; }
  bool operator!=(unequal_allocator const& x) const { return n_ != x.n_; }

  int n_;
};

template <class Stats> void check_stat(const Stats& s, bool full)
{
  if (full) {
    BOOST_TEST_NE(s.average, 0.0);
    if(s.variance) {
      BOOST_TEST_NE(s.deviation, 0.0);
    }
  }
  else {
    BOOST_TEST_EQ(s.average, 0.0);
    BOOST_TEST_EQ(s.variance, 0.0);
    BOOST_TEST_EQ(s.deviation, 0.0);
  }
}

template <class Stats> void check_stat(const Stats& s1, const Stats& s2)
{
  BOOST_TEST_EQ(s1.average, s2.average);
  BOOST_TEST_EQ(s1.variance, s2.variance);
  BOOST_TEST_EQ(s1.deviation, s2.deviation);
}

template <class Stats> void check_insertion_stats(const Stats& s, bool full)
{
  if (full) {
    BOOST_TEST_NE(s.count, 0);
  }
  else {
    BOOST_TEST_EQ(s.count, 0);
  }
  check_stat(s.probe_length, full);
}

template <class Stats>
void check_insertion_stats(const Stats& s1, const Stats& s2)
{
  BOOST_TEST_EQ(s1.count, s2.count);
  check_stat(s1.probe_length, s2.probe_length);
}

template <class Stats> void check_lookup_stats(const Stats& s, bool full)
{
  if (full) {
    BOOST_TEST_NE(s.count, 0);
  }
  else {
    BOOST_TEST_EQ(s.count, 0);
  }
  check_stat(s.probe_length, full);
  check_stat(s.num_comparisons, full);
}

template <class Stats>
void check_lookup_stats(const Stats& s1, const Stats& s2)
{
  BOOST_TEST_EQ(s1.count, s2.count);
  check_stat(s1.probe_length, s2.probe_length);
  check_stat(s1.num_comparisons, s2.num_comparisons);
}

template <class Stats> void check_container_stats(const Stats& s, bool full)
{
  check_insertion_stats(s.insertion, full);
  check_lookup_stats(s.successful_lookup, full);
  check_lookup_stats(s.unsuccessful_lookup, full);
}

template <class Stats>
void check_container_stats(const Stats& s1, const Stats& s2)
{
  check_insertion_stats(s1.insertion, s2.insertion);
  check_lookup_stats(s1.successful_lookup, s2.successful_lookup);
  check_lookup_stats(s1.unsuccessful_lookup, s2.unsuccessful_lookup);
}

template <class Container> void test_stats()
{
  using value_type = Container::value_type;
  using allocator_type = Container::allocator_type;
  using stats = Container::stats;
  const bool full = true, empty = false;

  Container        c;
  const Container& cc = c;

  stats s = cc.get_stats();
  check_container_stats(s, empty);

  test::reset_sequence();

#if defined(BOOST_UNORDERED_CFOA_TESTS)

  test::random_values<Container> l(10000, test::sequential);
  std::vector<value_type> v(l.begin(), l.end());
  thread_runner(v, [&c](boost::span<value_type> s) {
    for (auto const& x : s) {
      c.insert(x);
    }
  });

#else

  test::random_values<Container> v(10000, test::sequential);
  c.insert(v.begin(),v.end());

#endif

  s = cc.get_stats();
  check_insertion_stats(s.insertion, full);
  check_lookup_stats(s.successful_lookup, empty);
  check_lookup_stats(s.unsuccessful_lookup, full);

#if !defined(BOOST_UNORDERED_CFOA_TESTS)
  // Due to rehashing, may not hold in concurrent containers
  // because of insertion retries
  BOOST_TEST_GT(
    s.insertion.count, s.unsuccessful_lookup.count); 
#endif

  c.reset_stats();
  s = cc.get_stats();
  check_container_stats(s, empty);

  test::reset_sequence();

#if defined(BOOST_UNORDERED_CFOA_TESTS)

  test::random_values<Container> l2(15000, test::sequential);
  std::vector<value_type> v2(l2.begin(), l2.end());
  std::atomic<int> found = 0, not_found = 0;
  thread_runner(v2, [&cc, &found, &not_found](boost::span<value_type> s) {
    for (auto const& x : s) {
      if(cc.contains(test::get_key<Container>(x))) ++found;
      else                                         ++not_found;
    }
  });

#else

  test::random_values<Container> v2(15000, test::sequential);
  int found = 0, not_found = 0;
  for (const auto& x: v2) {
    if (cc.contains(test::get_key<Container>(x))) ++found;
    else                                          ++not_found;
  }

#endif

  s=cc.get_stats();
  check_lookup_stats(s.successful_lookup, full);
  check_lookup_stats(s.unsuccessful_lookup, full);
  BOOST_TEST_EQ(s.successful_lookup.count, found);
  BOOST_TEST_EQ(s.unsuccessful_lookup.count, not_found);

  c.reset_stats();
  s = cc.get_stats();
  check_container_stats(s, empty);

  test::reset_sequence();
  test::random_values<Container> v3(1000, test::sequential);
  c.clear();
  c.insert(v.begin(),v.end());
  c.insert(v.begin(),v.end()); // produces successful lookups

  s = cc.get_stats();
  Container c2 = std::move(c);
  check_container_stats(c.get_stats(), empty);
  check_container_stats(c2.get_stats(), s);

  Container c3(std::move(c2), allocator_type());
  check_container_stats(c2.get_stats(), empty);
  check_container_stats(c3.get_stats(), s);

  Container c4(std::move(c3), allocator_type(1));
  check_container_stats(c3.get_stats(), empty);
  check_insertion_stats(c4.get_stats().insertion, full);
  check_lookup_stats(c4.get_stats().successful_lookup, empty);
  check_lookup_stats(c4.get_stats().unsuccessful_lookup, empty);

  // TODO: move assignment
  // TODO: concurrent<->unordered interop
}

UNORDERED_AUTO_TEST (stats) {
#if defined(BOOST_UNORDERED_CFOA_TESTS)
  test_stats<
    boost::concurrent_flat_map<
      int, int, boost::hash<int>, std::equal_to<int>,
      unequal_allocator< std::pair< const int, int> >>>();
  test_stats<
    boost::concurrent_flat_set<
      int, boost::hash<int>, std::equal_to<int>, unequal_allocator<int>>>();
#elif defined(BOOST_UNORDERED_FOA_TESTS)
  test_stats<
    boost::unordered_flat_map<
      int, int, boost::hash<int>, std::equal_to<int>,
      unequal_allocator< std::pair< const int, int> >>>();
  test_stats<
    boost::unordered_flat_set<
      int, boost::hash<int>, std::equal_to<int>, unequal_allocator<int>>>();
  test_stats<
    boost::unordered_node_map<
      int, int, boost::hash<int>, std::equal_to<int>,
      unequal_allocator< std::pair< const int, int> >>>();
  test_stats<
    boost::unordered_node_set<
      int, boost::hash<int>, std::equal_to<int>, unequal_allocator<int>>>();
#else
  // Closed-addressing containers do not provide stats
#endif
}

RUN_TESTS()
