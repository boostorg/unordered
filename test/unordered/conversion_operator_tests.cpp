// Copyright 2026 Braden Ganetsky
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#if !defined(BOOST_UNORDERED_FOA_TESTS)
#error "This test is only for the FOA-style conatiners"
#endif

#include <boost/static_assert.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <boost/unordered/unordered_node_map.hpp>
#include <boost/unordered/unordered_node_set.hpp>

using flat_map = boost::unordered::unordered_flat_map<int, int>;
using flat_set = boost::unordered::unordered_flat_set<int>;
using node_map = boost::unordered::unordered_node_map<int, int>;
using node_set = boost::unordered::unordered_node_set<int>;

struct constrained_template_converter
{
  struct dummy
  {
  };
  template <class T, typename std::enable_if<
                       std::is_constructible<T, dummy>::value, int>::type = 0>
  operator T() const
  {
    return T{};
  }
};

// Check whether the corresponding CFOA container gets instantiated
BOOST_STATIC_ASSERT(
  (!std::is_constructible<flat_map, constrained_template_converter>::value));
BOOST_STATIC_ASSERT(
  (!std::is_constructible<flat_set, constrained_template_converter>::value));
BOOST_STATIC_ASSERT(
  (!std::is_constructible<node_map, constrained_template_converter>::value));
BOOST_STATIC_ASSERT(
  (!std::is_constructible<node_set, constrained_template_converter>::value));

#include <boost/unordered/concurrent_flat_map.hpp>
#include <boost/unordered/concurrent_flat_set.hpp>
#include <boost/unordered/concurrent_node_map.hpp>
#include <boost/unordered/concurrent_node_set.hpp>

using c_flat_map = boost::unordered::concurrent_flat_map<int, int>;
using c_flat_set = boost::unordered::concurrent_flat_set<int>;
using c_node_map = boost::unordered::concurrent_node_map<int, int>;
using c_node_set = boost::unordered::concurrent_node_set<int>;

template <class C> struct container_converter
{
  operator C() const { return {}; }
};

// Check whether the container can be constructed with an
// implicit conversion to the corresponding CFOA container
BOOST_STATIC_ASSERT(
  (std::is_constructible<flat_map, container_converter<c_flat_map> >::value));
BOOST_STATIC_ASSERT(
  (std::is_constructible<flat_set, container_converter<c_flat_set> >::value));
BOOST_STATIC_ASSERT(
  (std::is_constructible<node_map, container_converter<c_node_map> >::value));
BOOST_STATIC_ASSERT(
  (std::is_constructible<node_set, container_converter<c_node_set> >::value));

int main() { return 0; }
