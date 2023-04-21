// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "helpers.hpp"

#include <boost/unordered/concurrent_flat_map.hpp>

test::seed_t initialize_seed{2762556623};

using test::default_generator;
using test::limited_range;
using test::sequential;

using hasher = stateful_hash;
using key_equal = stateful_key_equal;
using allocator_type = stateful_allocator<std::pair<raii const, raii> >;

using map_type = boost::unordered::concurrent_flat_map<raii, raii, hasher,
  key_equal, allocator_type>;

using map_value_type = typename map_type::value_type;

namespace {
  template <class G> void copy_assign(G gen, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());

    // to test:
    // self-assign
    // propagation
    //

    // lhs empty, rhs empty
    {
      raii::reset_counts();

      map_type x(0, hasher(1), key_equal(2), allocator_type(3));
      map_type y;

      BOOST_TEST(x.empty());
      BOOST_TEST(y.empty());
      y = x;

      BOOST_TEST_EQ(raii::destructor, 0u);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::copy_constructor, 0u);

      BOOST_TEST_EQ(x.hash_function(), y.hash_function());
      BOOST_TEST_EQ(x.key_eq(), y.key_eq());
      BOOST_TEST(x.get_allocator() != y.get_allocator());
    }

    // lhs non-empty, rhs empty
    {
      raii::reset_counts();

      map_type x(0, hasher(1), key_equal(2), allocator_type(3));

      map_type y(values.begin(), values.end(), values.size());

      auto const old_cc = +raii::copy_constructor;
      auto const old_size = y.size();

      BOOST_TEST(x.empty());
      BOOST_TEST(!y.empty());
      y = x;

      BOOST_TEST_EQ(raii::destructor, 2 * old_size);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::copy_constructor, old_cc);

      BOOST_TEST_EQ(x.hash_function(), y.hash_function());
      BOOST_TEST_EQ(x.key_eq(), y.key_eq());
      BOOST_TEST(x.get_allocator() != y.get_allocator());
    }
    check_raii_counts();

    // lhs empty, rhs non-empty
    {
      raii::reset_counts();

      map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), allocator_type(3));

      map_type y;
      auto const old_cc = +raii::copy_constructor;

      BOOST_TEST(!x.empty());
      BOOST_TEST(y.empty());
      y = x;

      BOOST_TEST_EQ(raii::destructor, 0u);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::copy_constructor, old_cc + (2 * x.size()));

      BOOST_TEST_EQ(x.hash_function(), y.hash_function());
      BOOST_TEST_EQ(x.key_eq(), y.key_eq());
      BOOST_TEST(x.get_allocator() != y.get_allocator());

      test_matches_reference(y, reference_map);
    }
    check_raii_counts();

    // lhs non-empty, rhs non-empty
    {
      raii::reset_counts();

      map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), allocator_type(3));

      map_type y(values.begin(), values.end(), values.size());

      auto const old_size = y.size();
      auto const old_cc = +raii::copy_constructor;

      BOOST_TEST(!x.empty());
      BOOST_TEST(!y.empty());
      y = x;

      BOOST_TEST_EQ(raii::destructor, 2 * old_size);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::copy_constructor, old_cc + (2 * x.size()));

      BOOST_TEST_EQ(x.hash_function(), y.hash_function());
      BOOST_TEST_EQ(x.key_eq(), y.key_eq());
      BOOST_TEST(x.get_allocator() != y.get_allocator());
    }
    check_raii_counts();
  }

} // namespace

// clang-format off
UNORDERED_TEST(
  copy_assign,
  ((value_type_generator))
  ((default_generator)(sequential)(limited_range)))
// clang-format on

RUN_TESTS()
