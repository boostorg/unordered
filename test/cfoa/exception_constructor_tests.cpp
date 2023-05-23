// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "exception_helpers.hpp"

#include <boost/unordered/concurrent_flat_map.hpp>

using allocator_type = stateful_allocator<std::pair<raii const, raii> >;

using map_type = boost::unordered::concurrent_flat_map<raii, raii,
  stateful_hash, stateful_key_equal, allocator_type>;

namespace {
  test::seed_t initialize_seed(795610904);

  UNORDERED_AUTO_TEST (bucket_constructor) {
    raii::reset_counts();

    bool was_thrown = false;

    enable_exceptions();
    for (std::size_t i = 0; i < alloc_throw_threshold; ++i) {
      try {
        map_type m(128);
      } catch (...) {
        was_thrown = true;
      }
    }
    disable_exceptions();

    BOOST_TEST(was_thrown);
  }

  template <class G>
  void iterator_bucket_count_constructor(G gen, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });

    raii::reset_counts();

    bool was_thrown = false;

    enable_exceptions();
    try {
      map_type x(values.begin(), values.end(), 0, stateful_hash(1),
        stateful_key_equal(2), allocator_type(3));
    } catch (...) {
      was_thrown = true;
    }
    disable_exceptions();

    BOOST_TEST(was_thrown);
    check_raii_counts();
  }

  template <class G> void copy_constructor(G gen, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });

    {
      raii::reset_counts();

      bool was_thrown = false;

      try {
        map_type x(values.begin(), values.end(), 0);

        enable_exceptions();
        map_type y(x);
      } catch (...) {
        was_thrown = true;
      }
      disable_exceptions();

      BOOST_TEST(was_thrown);
      check_raii_counts();
    }

    {
      raii::reset_counts();

      bool was_thrown = false;

      try {
        map_type x(values.begin(), values.end(), 0);

        enable_exceptions();
        map_type y(x, allocator_type(4));
      } catch (...) {
        was_thrown = true;
      }
      disable_exceptions();

      BOOST_TEST(was_thrown);
      check_raii_counts();
    }
  }

  template <class G>
  void iterator_range_allocator_constructor(G gen, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });

    raii::reset_counts();

    bool was_thrown = false;

    enable_exceptions();
    try {
      map_type x(values.begin(), values.end(), allocator_type(3));
    } catch (...) {
      was_thrown = true;
    }
    disable_exceptions();

    BOOST_TEST(was_thrown);
    check_raii_counts();
  }

  template <class G> void move_constructor(G gen, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });

    {
      raii::reset_counts();

      bool was_thrown = false;

      try {
        map_type x(values.begin(), values.end(), 0);

        enable_exceptions();
        map_type y(std::move(x), allocator_type(4));
      } catch (...) {
        was_thrown = true;
      }
      disable_exceptions();

      BOOST_TEST(was_thrown);
      check_raii_counts();
    }
  }
} // namespace

using test::default_generator;
using test::limited_range;
using test::sequential;

// clang-format off
UNORDERED_TEST(
  iterator_bucket_count_constructor,
  ((exception_value_type_generator))
  ((default_generator)(sequential)(limited_range)))

UNORDERED_TEST(
  copy_constructor,
  ((exception_value_type_generator))
  ((default_generator)(sequential)))

UNORDERED_TEST(
  iterator_range_allocator_constructor,
  ((exception_value_type_generator))
  ((default_generator)(sequential)(limited_range)))

UNORDERED_TEST(
  move_constructor,
  ((exception_value_type_generator))
  ((default_generator)(sequential)))
// clang-format on

RUN_TESTS()
