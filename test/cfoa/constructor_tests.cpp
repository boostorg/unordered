// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "helpers.hpp"

#include <boost/unordered/concurrent_flat_map.hpp>

test::seed_t initialize_seed(4122023);

using test::default_generator;
using test::limited_range;
using test::sequential;

template <class T> struct soccc_allocator
{
  int x_ = -1;

  using value_type = T;

  soccc_allocator() = default;
  soccc_allocator(soccc_allocator const&) = default;
  soccc_allocator(soccc_allocator&&) = default;

  soccc_allocator(int const x) : x_{x} {}

  template <class U> soccc_allocator(soccc_allocator<U> const& rhs) : x_{rhs.x_}
  {
  }

  T* allocate(std::size_t n)
  {
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, std::size_t) { ::operator delete(p); }

  soccc_allocator select_on_container_copy_construction() const
  {
    return {x_ + 1};
  }

  bool operator==(soccc_allocator const& rhs) const { return x_ == rhs.x_; }
  bool operator!=(soccc_allocator const& rhs) const { return x_ != rhs.x_; }
};

using hasher = stateful_hash;
using key_equal = stateful_key_equal;
using allocator_type = std::allocator<std::pair<raii const, raii> >;

using map_type = boost::unordered::concurrent_flat_map<raii, raii, hasher,
  key_equal, allocator_type>;

UNORDERED_AUTO_TEST (default_constructor) {
  boost::unordered::concurrent_flat_map<raii, raii> x;
  BOOST_TEST(x.empty());
  BOOST_TEST_EQ(x.size(), 0u);
}

UNORDERED_AUTO_TEST (bucket_count_with_hasher_key_equal_and_allocator) {
  raii::reset_counts();
  {
    map_type x(0);

    BOOST_TEST(x.empty());
    BOOST_TEST_EQ(x.size(), 0u);
    BOOST_TEST_EQ(x.hash_function(), hasher());
    BOOST_TEST_EQ(x.key_eq(), key_equal());
  }

  {
    map_type x(0, hasher(1));

    BOOST_TEST(x.empty());
    BOOST_TEST_EQ(x.size(), 0u);
    BOOST_TEST_EQ(x.hash_function(), hasher(1));
    BOOST_TEST_EQ(x.key_eq(), key_equal());
  }

  {
    map_type x(0, hasher(1), key_equal(2));

    BOOST_TEST(x.empty());
    BOOST_TEST_EQ(x.size(), 0u);
    BOOST_TEST_EQ(x.hash_function(), hasher(1));
    BOOST_TEST_EQ(x.key_eq(), key_equal(2));
  }

  {
    map_type x(0, hasher(1), key_equal(2), allocator_type{});

    BOOST_TEST(x.empty());
    BOOST_TEST_EQ(x.size(), 0u);
    BOOST_TEST_EQ(x.hash_function(), hasher(1));
    BOOST_TEST_EQ(x.key_eq(), key_equal(2));
    BOOST_TEST(x.get_allocator() == allocator_type{});
  }
  raii::reset_counts();
}

UNORDERED_AUTO_TEST (soccc) {
  boost::unordered::concurrent_flat_map<raii, raii, hasher, key_equal,
    soccc_allocator<std::pair<raii const, raii> > >
    x;

  boost::unordered::concurrent_flat_map<raii, raii, hasher, key_equal,
    soccc_allocator<std::pair<raii const, raii> > >
    y(x);

  BOOST_TEST_EQ(y.hash_function(), x.hash_function());
  BOOST_TEST_EQ(y.key_eq(), x.key_eq());
  BOOST_TEST(y.get_allocator() != x.get_allocator());
}

namespace {
  template <class G> void from_iterator_range(G gen, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());
    raii::reset_counts();

    {
      map_type x(values.begin(), values.end());

      test_matches_reference(x, reference_map);
      BOOST_TEST_GT(x.size(), 0u);
      BOOST_TEST_LE(x.size(), values.size());
      BOOST_TEST_EQ(x.hash_function(), hasher());
      BOOST_TEST_EQ(x.key_eq(), key_equal());
      BOOST_TEST(x.get_allocator() == allocator_type{});
      if (rg == sequential) {
        BOOST_TEST_EQ(x.size(), values.size());
      }
      raii::reset_counts();
    }

    {
      map_type x(values.begin(), values.end(), 0);

      test_matches_reference(x, reference_map);
      BOOST_TEST_GT(x.size(), 0u);
      BOOST_TEST_LE(x.size(), values.size());
      BOOST_TEST_EQ(x.hash_function(), hasher());
      BOOST_TEST_EQ(x.key_eq(), key_equal());
      BOOST_TEST(x.get_allocator() == allocator_type{});
      if (rg == sequential) {
        BOOST_TEST_EQ(x.size(), values.size());
      }
      raii::reset_counts();
    }

    {
      map_type x(values.begin(), values.end(), 0, hasher(1));

      test_matches_reference(x, reference_map);
      BOOST_TEST_GT(x.size(), 0u);
      BOOST_TEST_LE(x.size(), values.size());
      BOOST_TEST_EQ(x.hash_function(), hasher(1));
      BOOST_TEST_EQ(x.key_eq(), key_equal());
      BOOST_TEST(x.get_allocator() == allocator_type{});
      if (rg == sequential) {
        BOOST_TEST_EQ(x.size(), values.size());
      }
      raii::reset_counts();
    }

    {
      map_type x(values.begin(), values.end(), 0, hasher(1), key_equal(2));

      test_matches_reference(x, reference_map);
      BOOST_TEST_GT(x.size(), 0u);
      BOOST_TEST_LE(x.size(), values.size());
      BOOST_TEST_EQ(x.hash_function(), hasher(1));
      BOOST_TEST_EQ(x.key_eq(), key_equal(2));
      BOOST_TEST(x.get_allocator() == allocator_type{});
      if (rg == sequential) {
        BOOST_TEST_EQ(x.size(), values.size());
      }
      raii::reset_counts();
    }

    {
      map_type x(values.begin(), values.end(), 0, hasher(1), key_equal(2),
        allocator_type{});

      test_matches_reference(x, reference_map);
      BOOST_TEST_GT(x.size(), 0u);
      BOOST_TEST_LE(x.size(), values.size());
      BOOST_TEST_EQ(x.hash_function(), hasher(1));
      BOOST_TEST_EQ(x.key_eq(), key_equal(2));
      BOOST_TEST(x.get_allocator() == allocator_type{});
      if (rg == sequential) {
        BOOST_TEST_EQ(x.size(), values.size());
      }
      raii::reset_counts();
    }
  }

  template <class G> void copy_constructor(G gen, test::random_generator rg)
  {
    {
      map_type x(0, hasher(1), key_equal(2), allocator_type{});
      map_type y(x);

      BOOST_TEST_EQ(y.size(), x.size());
      BOOST_TEST_EQ(y.hash_function(), x.hash_function());
      BOOST_TEST_EQ(y.key_eq(), x.key_eq());
      BOOST_TEST(y.get_allocator() == x.get_allocator());
    }

    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());
    raii::reset_counts();

    {
      map_type x(values.begin(), values.end(), 0, hasher(1), key_equal(2),
        allocator_type{});

      thread_runner(
        values, [&x, &reference_map](
                  boost::span<typename decltype(values)::value_type> s) {
          (void)s;
          map_type y(x);

          test_matches_reference(x, reference_map);
          test_matches_reference(y, reference_map);
          BOOST_TEST_EQ(y.size(), x.size());
          BOOST_TEST_EQ(y.hash_function(), x.hash_function());
          BOOST_TEST_EQ(y.key_eq(), x.key_eq());
          BOOST_TEST(y.get_allocator() == x.get_allocator());
        });
    }
  }

  template <class G>
  void copy_constructor_with_insertion(G gen, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());
    raii::reset_counts();

    {
      map_type x(0, hasher(1), key_equal(2), allocator_type{});

      auto f = [&x, &values] {
        std::this_thread::sleep_for(std::chrono::milliseconds(75));
        for (auto const& val : values) {
          x.insert(val);
        }
      };

      std::thread t1(f);
      std::thread t2(f);

      thread_runner(
        values, [&x, &reference_map, &values, rg](
                  boost::span<typename decltype(values)::value_type> s) {
          (void)s;
          map_type y(x);

          BOOST_TEST_LE(y.size(), values.size());
          BOOST_TEST_EQ(y.hash_function(), x.hash_function());
          BOOST_TEST_EQ(y.key_eq(), x.key_eq());
          BOOST_TEST(y.get_allocator() == x.get_allocator());

          x.visit_all([&reference_map, rg](
                        typename map_type::value_type const& val) {
            BOOST_TEST(reference_map.contains(val.first));
            if (rg == sequential) {
              BOOST_TEST_EQ(val.second, reference_map.find(val.first)->second);
            }
          });
        });

      t1.join();
      t2.join();
    }
  }

} // namespace

// clang-format off
UNORDERED_TEST(
  from_iterator_range,
  ((value_type_generator))
  ((default_generator)(sequential)(limited_range)))

UNORDERED_TEST(
  copy_constructor,
  ((value_type_generator))
  ((default_generator)(sequential)(limited_range)))

UNORDERED_TEST(
  copy_constructor_with_insertion,
  ((value_type_generator))
  ((default_generator)(sequential)(limited_range)))
// clang-format on

RUN_TESTS()
