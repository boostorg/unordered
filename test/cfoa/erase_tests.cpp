// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "helpers.hpp"

#include <boost/unordered/concurrent_flat_map.hpp>

#include <boost/core/ignore_unused.hpp>

namespace {
  test::seed_t initialize_seed(3292023);

  struct lvalue_eraser_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_erased{0};
      auto const old_size = x.size();

      auto const old_dc = +raii::default_constructor;
      auto const old_cc = +raii::copy_constructor;
      auto const old_mc = +raii::move_constructor;

      auto const old_d = +raii::destructor;

      BOOST_TEST_EQ(raii::default_constructor + raii::copy_constructor +
                      raii::move_constructor,
        raii::destructor + 2 * x.size());

      thread_runner(values, [&values, &num_erased, &x](boost::span<T>) {
        for (auto const& k : values) {
          auto count = x.erase(k.first);
          num_erased += count;
          BOOST_TEST_LE(count, 1u);
          BOOST_TEST_GE(count, 0u);
        }
      });

      BOOST_TEST_EQ(raii::default_constructor, old_dc);
      BOOST_TEST_EQ(raii::copy_constructor, old_cc);
      BOOST_TEST_EQ(raii::move_constructor, old_mc);

      BOOST_TEST_EQ(raii::destructor, old_d + 2 * old_size);

      BOOST_TEST_EQ(x.size(), 0u);
      BOOST_TEST(x.empty());
      BOOST_TEST_EQ(num_erased, old_size);
    }
  } lvalue_eraser;

  template <class X, class G, class F>
  void erase(X*, G gen, F eraser, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());
    raii::reset_counts();

    {
      X x;

      x.insert(values.begin(), values.end());

      BOOST_TEST_EQ(x.size(), reference_map.size());

      using value_type = typename X::value_type;
      BOOST_TEST_EQ(x.size(), x.visit_all([&](value_type const& kv) {
        BOOST_TEST(reference_map.contains(kv.first));
        if (rg == test::sequential) {
          BOOST_TEST_EQ(kv.second, reference_map[kv.first]);
        }
      }));

      eraser(values, x);
    }

    BOOST_TEST_GE(raii::default_constructor, 0u);
    BOOST_TEST_GE(raii::copy_constructor, 0u);
    BOOST_TEST_GE(raii::move_constructor, 0u);
    BOOST_TEST_GT(raii::destructor, 0u);

    BOOST_TEST_EQ(raii::default_constructor + raii::copy_constructor +
                    raii::move_constructor,
      raii::destructor);
  }

  boost::unordered::concurrent_flat_map<raii, raii>* map;
  //   boost::unordered::concurrent_flat_map<raii, raii, transparent_hash,
  //     transparent_key_equal>* transparent_map;

} // namespace

using test::default_generator;
using test::limited_range;
using test::sequential;

// clang-format off
UNORDERED_TEST(
  erase,
  ((map))
  ((value_type_generator)(init_type_generator))
  ((lvalue_eraser))
  ((default_generator)(sequential)(limited_range)))

// clang-format on

RUN_TESTS()
