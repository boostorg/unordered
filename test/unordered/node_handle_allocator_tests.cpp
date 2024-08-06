// Copyright (C) 2024 Joaquin M Lopez Munoz
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifdef BOOST_UNORDERED_CFOA_TESTS
#include <boost/unordered/concurrent_node_map.hpp>
#include <boost/unordered/concurrent_node_set.hpp>
#else
#include "../helpers/unordered.hpp"
#endif

#include "../helpers/test.hpp"

#include <boost/core/allocator_access.hpp>
#include <memory>
#include <type_traits>

namespace {
  template <class T> struct pocma_allocator
  {
    int x_ = -1;

    using value_type = T;
    using propagate_on_container_move_assignment = std::true_type;

    pocma_allocator() = default;
    pocma_allocator(pocma_allocator const&) = default;
    pocma_allocator(int const x) : x_{x} {}

    template <class U>
    pocma_allocator(pocma_allocator<U> const& rhs) : x_{rhs.x_}
    {
    }

    pocma_allocator& operator=(pocma_allocator const&) = default;

    T* allocate(std::size_t n)
    {
      return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t) { ::operator delete(p); }

    bool operator==(pocma_allocator const& rhs) const { return x_ == rhs.x_; }
    bool operator!=(pocma_allocator const& rhs) const { return x_ != rhs.x_; }
  };

  template <typename Container, typename Allocator>
  struct replace_allocator_impl;

  template <typename Container, typename Allocator>
  using replace_allocator = 
    typename replace_allocator_impl<Container, Allocator>::type;

  template <
    typename K, typename H, typename P, typename A,
    template <typename, typename, typename, typename> class Set,
    typename Allocator
  >
  struct replace_allocator_impl<Set<K, H, P, A>, Allocator>
  {
    using type = Set<
      K, H, P, boost::allocator_rebind_t<Allocator, K> >;
  };

  template <
    typename K, typename H, typename T, typename P, typename A,
    template <typename, typename, typename, typename, typename> class Map,
    typename Allocator
  >
  struct replace_allocator_impl<Map<K, T, H, P, A>, Allocator>
  {
    using type = Map<
      K, T, H, P,
      boost::allocator_rebind_t<Allocator, std::pair<K const, T> > >;
  };

  template<typename X, typename Allocator>
  void node_handle_allocator_tests(
    X*, std::pair<Allocator, Allocator> allocators)
  {
    using value_type = typename X::value_type;
    using replaced_allocator_container = replace_allocator<X, Allocator>;
    using node_type = typename replaced_allocator_container::node_type;

    replaced_allocator_container x1(allocators.first);
    node_type nh;

    x1.emplace(value_type());
    nh = x1.extract(0);

    BOOST_TEST(!nh.empty());
    BOOST_TEST(nh.get_allocator() == x1.get_allocator());

    replaced_allocator_container x2(allocators.second);

    x2.emplace(value_type());
    nh = x2.extract(0);

    BOOST_TEST(!nh.empty());
    BOOST_TEST(nh.get_allocator() == x2.get_allocator());
  }

  std::pair<std::allocator<int>, std::allocator<int> > test_std_allocators;
  std::pair<
    pocma_allocator<int>, pocma_allocator<int> > test_pocma_allocators(5,6);

#if defined(BOOST_UNORDERED_FOA_TESTS)
  boost::unordered_node_map<int, int>* test_map;
  boost::unordered_node_set<int>* test_set;
#elif defined(BOOST_UNORDERED_CFOA_TESTS)
  boost::concurrent_node_map<int, int>* test_map;
  boost::concurrent_node_set<int>* test_set;
#else
  boost::unordered_map<int, int>* test_map;
  boost::unordered_set<int>* test_set;
#endif
} // namespace

// clang-format off
UNORDERED_TEST(
  node_handle_allocator_tests,
  ((test_map)(test_set))
  ((test_std_allocators)(test_pocma_allocators)))
// clang-format on

RUN_TESTS()
