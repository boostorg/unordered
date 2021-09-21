
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

#include <boost/container/scoped_allocator.hpp>

#include <boost/interprocess/allocators/node_allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include <cstdint>
#include <utility>
#include <vector>

// This test is based on a user-submitted issue found here:
// https://github.com/boostorg/unordered/issues/22
//

namespace scoped_allocator {
  namespace bi = boost::interprocess;

  template <typename T>
  using scoped_alloc_t = boost::container::scoped_allocator_adaptor<T>;

  template <class T>
  using alloc_type =
    bi::node_allocator<T, bi::managed_shared_memory::segment_manager>;

  void use_scoped_allocator()
  {
    using way_type = std::vector<uint64_t, alloc_type<uint64_t> >;

    using waystore_pair_type = std::pair<uint64_t, way_type>;

    using waystore_pair_allocator_type = alloc_type<waystore_pair_type>;

    using map_type =
      boost::unordered_map<const uint64_t, way_type, std::hash<uint64_t>,
        std::equal_to<uint64_t>, scoped_alloc_t<waystore_pair_allocator_type> >;

    bi::managed_shared_memory s(
      bi::create_only, "unordered-shared-mem-test", 65536);

    map_type map(s.get_segment_manager());

    for (unsigned i = 0; i < 10; ++i) {
      auto const way = {1, 2, 3, 4};
      map.emplace(std::piecewise_construct, std::forward_as_tuple(i),
        std::forward_as_tuple(way.begin(), way.end()));
    }

    BOOST_TEST(map.size() == 10);

    bi::shared_memory_object::remove("unordered-shared-mem-test");
  }

  UNORDERED_TEST(use_scoped_allocator, (()))

} // namespace scoped_allocator

RUN_TESTS()
