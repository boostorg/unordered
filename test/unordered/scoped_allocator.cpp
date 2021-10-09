
// Copyright 2021 Christian Mazakas.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off
#include "../helpers/prefix.hpp"
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "../helpers/postfix.hpp"
// clang-format on

#include "../helpers/test.hpp"

#include <boost/config.hpp>
#include <boost/cstdint.hpp>

#include <boost/container/scoped_allocator.hpp>
#include <boost/container/uses_allocator.hpp>

#include <boost/core/ignore_unused.hpp>

#include <boost/interprocess/allocators/node_allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include <utility>
#include <vector>

// This test is based on a user-submitted issue found here:
// https://github.com/boostorg/unordered/issues/22
//

namespace bi = boost::interprocess;

template <typename T> struct node_alloc
{
  typedef bi::node_allocator<T, bi::managed_shared_memory::segment_manager>
    type;
};

typedef std::vector<boost::uint64_t, node_alloc<boost::uint64_t>::type>
  vector_type;

typedef std::pair<boost::uint64_t, vector_type> pair_type;

typedef boost::container::scoped_allocator_adaptor<node_alloc<pair_type>::type,
  node_alloc<boost::uint64_t>::type>
  allocator_type;

typedef boost::unordered_map<const boost::uint64_t, vector_type,
  boost::hash<boost::uint64_t>, std::equal_to<boost::uint64_t>, allocator_type>
  map_type;

UNORDERED_AUTO_TEST (scoped_allocator) {
#if BOOST_CXX_VERSION > 199711L
  bi::managed_shared_memory s(
    bi::create_only, "unordered-shared-mem-test", 65536);

  allocator_type alloc(node_alloc<pair_type>::type(s.get_segment_manager()),
    node_alloc<boost::uint64_t>::type(s.get_segment_manager()));

  map_type map(alloc);

  for (unsigned i = 0; i < 10; ++i) {
    boost::ignore_unused(map[i]);
  }

  BOOST_TEST(map.size() == 10);

  bi::shared_memory_object::remove("unordered-shared-mem-test");
#endif
}

RUN_TESTS()
