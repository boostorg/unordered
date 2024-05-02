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
#include <cstring>

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

bool exact_same(double x, double y)
{
  return std::memcmp(
    reinterpret_cast<void*>(&x), reinterpret_cast<void*>(&y),
    sizeof(double))==0;
}

bool not_exact_same(double x, double y)
{
  return !exact_same(x, y);
}

template <class Stats> void check_stat(const Stats& s, bool full)
{
  if (full) {
    BOOST_TEST_GT(s.average, 0.0);
    if(not_exact_same(s.variance, 0.0)) {
      BOOST_TEST_GT(s.variance, 0.0);
      BOOST_TEST_GT(s.deviation, 0.0);
    }
  }
  else {
    BOOST_TEST(exact_same(s.average, 0.0));
    BOOST_TEST(exact_same(s.variance, 0.0));
    BOOST_TEST(exact_same(s.deviation, 0.0));
  }
}

template <class Stats> void check_stat(const Stats& s1, const Stats& s2)
{
  BOOST_TEST(exact_same(s1.average, s2.average));
  BOOST_TEST(exact_same(s1.variance, s2.variance));
  BOOST_TEST(exact_same(s1.deviation, s2.deviation));
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

template <class Container> void insert_n(Container& c, std::size_t n)
{
#if defined(BOOST_UNORDERED_CFOA_TESTS)
  using value_type = typename Container::value_type;

  test::reset_sequence();
  test::random_values<Container> l(n, test::sequential);
  std::vector<value_type> v(l.begin(), l.end());
  thread_runner(v, [&c](boost::span<value_type> sp) {
    for (auto const& x : sp) {
      c.insert(x);
    }
  });
#else
  test::reset_sequence();
  test::random_values<Container> l(n, test::sequential);
  c.insert(l.begin(), l.end());
#endif
}

template <class Container> void test_stats()
{
  using allocator_type = typename Container::allocator_type;
  using stats = typename Container::stats;
  const bool full = true, empty = false;

  Container        c;
  const Container& cc = c;

  // Stats initially empty
  stats s = cc.get_stats(); // using cc -> get_stats() is const
  check_container_stats(s, empty);

  // Stats after insertion
  insert_n(c, 10000);
  s = cc.get_stats();
  check_insertion_stats(s.insertion, full); // insertions happened
  check_lookup_stats(s.successful_lookup, empty); // no duplicate values
  check_lookup_stats(s.unsuccessful_lookup, full); // from insertion

#if !defined(BOOST_UNORDERED_CFOA_TESTS)
  // Inequality due to rehashing
  // May not hold in concurrent containers because of insertion retries
  BOOST_TEST_GT(
    s.insertion.count, s.unsuccessful_lookup.count); 
#endif

  // resets_stats() actually clears stats
  c.reset_stats();
  check_container_stats(cc.get_stats(), empty);

  // Stats after lookup

  test::reset_sequence();

#if defined(BOOST_UNORDERED_CFOA_TESTS)

  using value_type = typename Container::value_type;

  test::random_values<Container> l2(15000, test::sequential);
  std::vector<value_type> v2(l2.begin(), l2.end());
  std::atomic<int> found{0}, not_found{0};
  thread_runner(v2, [&cc, &found, &not_found](boost::span<value_type> sp) {
    for (auto const& x : sp) {
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

  // As many [un]successful lookups as recorded externally
  s=cc.get_stats();
  check_lookup_stats(s.successful_lookup, full);
  check_lookup_stats(s.unsuccessful_lookup, full);
  BOOST_TEST_EQ(s.successful_lookup.count, found);
  BOOST_TEST_EQ(s.unsuccessful_lookup.count, not_found);

  c.reset_stats();
  s = cc.get_stats();
  check_container_stats(s, empty);

  // Move constructor tests

  c.clear();
  insert_n(c, 1000);
  insert_n(c, 1000); // produces successful lookups

  // Move contructor
  // Stats transferred to target and reset in source
  s = cc.get_stats();
  Container c2 = std::move(c);
  check_container_stats(c.get_stats(), empty);
  check_container_stats(c2.get_stats(), s);

  // Move constructor with equal allocator
  // Stats transferred to target and reset in source
  Container c3(std::move(c2), allocator_type());
  check_container_stats(c2.get_stats(), empty);
  check_container_stats(c3.get_stats(), s);

  // Move constructor with unequal allocator
  // Target only has insertions, stats reset in source
  Container c4(std::move(c3), allocator_type(1));
  check_container_stats(c3.get_stats(), empty);
  check_insertion_stats(c4.get_stats().insertion, full);
  check_lookup_stats(c4.get_stats().successful_lookup, empty);
  check_lookup_stats(c4.get_stats().unsuccessful_lookup, empty);

  // Move assignment tests

  // Move assignment with equal allocator
  // Stats transferred to target and reset in source
  Container c5, c6;
  insert_n(c5,1000);
  insert_n(c5,1000); // produces successful lookups
  insert_n(c6,500);
  insert_n(c6,500); // produces successful lookups
  s = c5.get_stats();
  check_container_stats(s, full);
  check_container_stats(c6.get_stats(), full);
  c6 = std::move(c5);
  check_container_stats(c5.get_stats(), empty);
  check_container_stats(c6.get_stats(), s);

  // Move assignment with unequal allocator
  // Target only has insertions (if reset previously), stats reset in source
  Container c7(allocator_type(1));
  insert_n(c7,250);
  insert_n(c7,250); // produces successful lookups
  check_container_stats(c7.get_stats(), full);
  c7.reset_stats();
  c7 = std::move(c6);
  check_container_stats(c6.get_stats(), empty);
  check_insertion_stats(c7.get_stats().insertion, full);
  check_lookup_stats(c7.get_stats().successful_lookup, empty);
  check_lookup_stats(c7.get_stats().unsuccessful_lookup, empty);

  // TODO: concurrent<->unordered interop
}

UNORDERED_AUTO_TEST (stats_) {
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
