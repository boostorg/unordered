// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "helpers.hpp"

#include <boost/unordered/concurrent_flat_map.hpp>

#if defined(__clang__) && defined(__has_warning)

#if __has_warning("-Wself-assign-overloaded")
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif

#if __has_warning("-Wself-move")
#pragma clang diagnostic ignored "-Wself-move"
#endif

#endif /* defined(__clang__) && defined(__has_warning) */

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

template <class T> struct pocca_allocator
{
  using propagate_on_container_copy_assignment = std::true_type;

  int x_ = -1;

  using value_type = T;

  pocca_allocator() = default;
  pocca_allocator(pocca_allocator const&) = default;
  pocca_allocator(pocca_allocator&&) = default;

  pocca_allocator(int const x) : x_{x} {}

  pocca_allocator& operator=(pocca_allocator const& rhs)
  {
    if (this != &rhs) {
      x_ = rhs.x_;
    }
    return *this;
  }

  template <class U> pocca_allocator(pocca_allocator<U> const& rhs) : x_{rhs.x_}
  {
  }

  T* allocate(std::size_t n)
  {
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, std::size_t) { ::operator delete(p); }

  bool operator==(pocca_allocator const& rhs) const { return x_ == rhs.x_; }
  bool operator!=(pocca_allocator const& rhs) const { return x_ != rhs.x_; }
};

template <class T> struct pocma_allocator
{
  using propagate_on_container_move_assignment = std::true_type;

  int x_ = -1;

  using value_type = T;

  pocma_allocator() = default;
  pocma_allocator(pocma_allocator const&) = default;
  pocma_allocator(pocma_allocator&&) = default;

  pocma_allocator(int const x) : x_{x} {}

  pocma_allocator& operator=(pocma_allocator const& rhs)
  {
    if (this != &rhs) {
      x_ = rhs.x_;
    }
    return *this;
  }

  template <class U> pocma_allocator(pocma_allocator<U> const& rhs) : x_{rhs.x_}
  {
  }

  T* allocate(std::size_t n)
  {
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, std::size_t) { ::operator delete(p); }

  bool operator==(pocma_allocator const& rhs) const { return x_ == rhs.x_; }
  bool operator!=(pocma_allocator const& rhs) const { return x_ != rhs.x_; }
};

namespace {
  template <class G> void copy_assign(G gen, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());

    // lhs empty, rhs empty
    {
      raii::reset_counts();

      map_type x(0, hasher(1), key_equal(2), allocator_type(3));

      thread_runner(values, [&x](boost::span<map_value_type> s) {
        (void)s;

        map_type y;

        BOOST_TEST(x.empty());
        BOOST_TEST(y.empty());

        y = x;

        BOOST_TEST_EQ(x.hash_function(), y.hash_function());
        BOOST_TEST_EQ(x.key_eq(), y.key_eq());
        BOOST_TEST(x.get_allocator() != y.get_allocator());
      });

      BOOST_TEST_EQ(raii::destructor, 0u);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::copy_constructor, 0u);
    }

    // lhs non-empty, rhs empty
    {
      raii::reset_counts();

      map_type x(0, hasher(1), key_equal(2), allocator_type(3));

      auto const old_size = reference_map.size();

      thread_runner(values, [&x, &values](boost::span<map_value_type> s) {
        (void)s;

        map_type y(values.begin(), values.end(), values.size());

        BOOST_TEST(x.empty());
        BOOST_TEST(!y.empty());

        y = x;

        BOOST_TEST_EQ(x.hash_function(), y.hash_function());
        BOOST_TEST_EQ(x.key_eq(), y.key_eq());
        BOOST_TEST(x.get_allocator() != y.get_allocator());

        BOOST_TEST(y.empty());
      });

      BOOST_TEST_EQ(raii::destructor, num_threads * (2 * old_size));
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(
        raii::copy_constructor, num_threads * 2 * reference_map.size());
    }
    check_raii_counts();

    // lhs empty, rhs non-empty
    {
      raii::reset_counts();

      map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), allocator_type(3));

      auto const old_cc = +raii::copy_constructor;

      thread_runner(
        values, [&x, &reference_map](boost::span<map_value_type> s) {
          (void)s;

          map_type y;

          BOOST_TEST(!x.empty());
          BOOST_TEST(y.empty());

          y = x;

          BOOST_TEST_EQ(x.hash_function(), y.hash_function());
          BOOST_TEST_EQ(x.key_eq(), y.key_eq());
          BOOST_TEST(x.get_allocator() != y.get_allocator());

          test_matches_reference(y, reference_map);
        });

      BOOST_TEST_EQ(raii::destructor, num_threads * 2 * x.size());
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(
        raii::copy_constructor, old_cc + (num_threads * 2 * x.size()));
    }
    check_raii_counts();

    // lhs non-empty, rhs non-empty
    {
      raii::reset_counts();

      map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), allocator_type(3));

      auto const old_size = x.size();
      auto const old_cc = +raii::copy_constructor;

      thread_runner(values, [&x, &values](boost::span<map_value_type> s) {
        (void)s;

        map_type y(values.begin(), values.end(), values.size());

        BOOST_TEST(!x.empty());
        BOOST_TEST(!y.empty());

        y = x;

        BOOST_TEST_EQ(x.hash_function(), y.hash_function());
        BOOST_TEST_EQ(x.key_eq(), y.key_eq());
        BOOST_TEST(x.get_allocator() != y.get_allocator());
      });

      BOOST_TEST_EQ(raii::destructor, 2 * num_threads * 2 * old_size);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(
        raii::copy_constructor, old_cc + (2 * num_threads * 2 * x.size()));
    }
    check_raii_counts();

    // self-assign
    {
      raii::reset_counts();

      map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), allocator_type(3));

      auto const old_cc = +raii::copy_constructor;

      thread_runner(
        values, [&x, &reference_map](boost::span<map_value_type> s) {
          (void)s;

          BOOST_TEST(!x.empty());

          x = x;

          BOOST_TEST_EQ(x.hash_function(), hasher(1));
          BOOST_TEST_EQ(x.key_eq(), key_equal(2));
          BOOST_TEST(x.get_allocator() == allocator_type(3));

          test_matches_reference(x, reference_map);
        });

      BOOST_TEST_EQ(raii::destructor, 0u);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::copy_constructor, old_cc);
    }
    check_raii_counts();

    // propagation
    {
      using pocca_allocator_type =
        pocca_allocator<std::pair<const raii, raii> >;

      using pocca_map_type = boost::unordered::concurrent_flat_map<raii, raii,
        hasher, key_equal, pocca_allocator_type>;

      raii::reset_counts();

      pocca_map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), pocca_allocator_type(3));

      auto const old_size = x.size();
      auto const old_cc = +raii::copy_constructor;

      thread_runner(values, [&x, &values](boost::span<map_value_type> s) {
        (void)s;

        pocca_map_type y(values.begin(), values.end(), values.size());

        BOOST_TEST(!x.empty());
        BOOST_TEST(!y.empty());

        BOOST_TEST(x.get_allocator() != y.get_allocator());

        y = x;

        BOOST_TEST_EQ(x.hash_function(), y.hash_function());
        BOOST_TEST_EQ(x.key_eq(), y.key_eq());
        BOOST_TEST(x.get_allocator() == y.get_allocator());
      });

      BOOST_TEST_EQ(raii::destructor, 2 * num_threads * 2 * old_size);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(
        raii::copy_constructor, old_cc + (2 * num_threads * 2 * x.size()));
    }
    check_raii_counts();
  }

  template <class G> void move_assign(G gen, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());

    // move assignment has more complex requirements than copying
    // equal allocators:
    // lhs empty, rhs non-empty
    // lhs non-empty, rhs empty
    // lhs non-empty, rhs non-empty
    //
    // unequal allocators:
    // lhs non-empty, rhs non-empty
    //
    // pocma
    // self move-assign

    // lhs empty, rhs empty
    {
      raii::reset_counts();

      map_type x(0, hasher(1), key_equal(2), allocator_type(3));

      std::atomic<unsigned> num_transfers{0};

      thread_runner(
        values, [&x, &num_transfers](boost::span<map_value_type> s) {
          (void)s;

          map_type y(0, hasher(2), key_equal(1), allocator_type(3));

          BOOST_TEST(x.empty());
          BOOST_TEST(y.empty());
          BOOST_TEST(x.get_allocator() == y.get_allocator());

          y = std::move(x);
          if (y.hash_function() == hasher(1)) {
            ++num_transfers;
            BOOST_TEST_EQ(y.key_eq(), key_equal(2));
          } else {
            BOOST_TEST_EQ(y.hash_function(), hasher(2));
            BOOST_TEST_EQ(y.key_eq(), key_equal(1));
          }

          BOOST_TEST_EQ(x.hash_function(), hasher(2));
          BOOST_TEST_EQ(x.key_eq(), key_equal(1));
          BOOST_TEST(x.get_allocator() == y.get_allocator());
        });

      BOOST_TEST_EQ(num_transfers, 1u);

      BOOST_TEST_EQ(raii::destructor, 0u);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::copy_constructor, 0u);
    }

    // lhs non-empty, rhs empty
    {
      raii::reset_counts();

      map_type x(0, hasher(1), key_equal(2), allocator_type(3));

      std::atomic<unsigned> num_transfers{0};

      thread_runner(
        values, [&x, &values, &num_transfers](boost::span<map_value_type> s) {
          (void)s;

          map_type y(values.begin(), values.end(), values.size(), hasher(2),
            key_equal(1), allocator_type(3));

          BOOST_TEST(x.empty());
          BOOST_TEST(!y.empty());
          BOOST_TEST(x.get_allocator() == y.get_allocator());

          y = std::move(x);
          if (y.hash_function() == hasher(1)) {
            ++num_transfers;
            BOOST_TEST_EQ(y.key_eq(), key_equal(2));
          } else {
            BOOST_TEST_EQ(y.hash_function(), hasher(2));
            BOOST_TEST_EQ(y.key_eq(), key_equal(1));
          }

          BOOST_TEST_EQ(x.hash_function(), hasher(2));
          BOOST_TEST_EQ(x.key_eq(), key_equal(1));
          BOOST_TEST(x.get_allocator() == y.get_allocator());

          BOOST_TEST(y.empty());
        });

      BOOST_TEST_EQ(num_transfers, 1u);

      BOOST_TEST_EQ(raii::destructor, num_threads * 2 * reference_map.size());
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(
        raii::copy_constructor, num_threads * 2 * reference_map.size());
    }
    check_raii_counts();

    // lhs empty, rhs non-empty
    {
      raii::reset_counts();

      map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), allocator_type(3));

      auto const old_cc = +raii::copy_constructor;
      auto const old_mc = +raii::move_constructor;
      std::atomic<unsigned> num_transfers{0};

      thread_runner(values,
        [&x, &reference_map, &num_transfers](boost::span<map_value_type> s) {
          (void)s;

          map_type y(allocator_type(3));

          BOOST_TEST(y.empty());
          BOOST_TEST(x.get_allocator() == y.get_allocator());

          y = std::move(x);
          if (!y.empty()) {
            ++num_transfers;
            test_matches_reference(y, reference_map);

            BOOST_TEST_EQ(y.hash_function(), hasher(1));
            BOOST_TEST_EQ(y.key_eq(), key_equal(2));
          } else {
            BOOST_TEST_EQ(y.hash_function(), hasher());
            BOOST_TEST_EQ(y.key_eq(), key_equal());
          }

          BOOST_TEST(x.empty());

          BOOST_TEST_EQ(x.hash_function(), hasher());
          BOOST_TEST_EQ(x.key_eq(), key_equal());
          BOOST_TEST(x.get_allocator() == y.get_allocator());
        });

      BOOST_TEST_EQ(num_transfers, 1u);

      BOOST_TEST_EQ(raii::destructor, 2 * reference_map.size());
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::copy_constructor, old_cc);
      BOOST_TEST_EQ(raii::move_constructor, old_mc);
    }
    check_raii_counts();

    // lhs non-empty, rhs non-empty
    {
      raii::reset_counts();

      map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), allocator_type(3));

      auto const old_size = x.size();
      auto const old_cc = +raii::copy_constructor;
      auto const old_mc = +raii::move_constructor;

      std::atomic<unsigned> num_transfers{0};

      thread_runner(values, [&x, &values, &num_transfers, &reference_map](
                              boost::span<map_value_type> s) {
        (void)s;

        map_type y(values.begin(), values.end(), values.size(), hasher(2),
          key_equal(1), allocator_type(3));

        BOOST_TEST(!y.empty());
        BOOST_TEST(x.get_allocator() == y.get_allocator());

        y = std::move(x);
        if (y.hash_function() == hasher(1)) {
          ++num_transfers;
          test_matches_reference(y, reference_map);

          BOOST_TEST_EQ(y.key_eq(), key_equal(2));
        } else {
          BOOST_TEST_EQ(y.hash_function(), hasher(2));
          BOOST_TEST_EQ(y.key_eq(), key_equal(1));
        }

        BOOST_TEST(x.empty());

        BOOST_TEST_EQ(x.hash_function(), hasher(2));
        BOOST_TEST_EQ(x.key_eq(), key_equal(1));
        BOOST_TEST(x.get_allocator() == y.get_allocator());
      });

      BOOST_TEST_EQ(num_transfers, 1u);

      BOOST_TEST_EQ(
        raii::destructor, 2 * old_size + num_threads * 2 * old_size);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::move_constructor, old_mc);
      BOOST_TEST_EQ(raii::copy_constructor,
        old_cc + (num_threads * 2 * reference_map.size()));
    }
    check_raii_counts();

    // lhs non-empty, rhs non-empty, unequal allocators, no propagation
    {
      raii::reset_counts();

      map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), allocator_type(3));

      auto const old_size = x.size();
      auto const old_cc = +raii::copy_constructor;
      auto const old_mc = +raii::move_constructor;

      std::atomic<unsigned> num_transfers{0};

      thread_runner(values, [&x, &values, &num_transfers, &reference_map](
                              boost::span<map_value_type> s) {
        (void)s;

        map_type y(values.begin(), values.end(), values.size(), hasher(2),
          key_equal(1), allocator_type(13));

        BOOST_TEST(
          !boost::allocator_is_always_equal<allocator_type>::type::value);

        BOOST_TEST(!boost::allocator_propagate_on_container_move_assignment<
                   allocator_type>::type::value);

        BOOST_TEST(!y.empty());
        BOOST_TEST(x.get_allocator() != y.get_allocator());

        y = std::move(x);
        if (y.hash_function() == hasher(1)) {
          ++num_transfers;
          test_matches_reference(y, reference_map);

          BOOST_TEST_EQ(y.key_eq(), key_equal(2));
        } else {
          BOOST_TEST_EQ(y.hash_function(), hasher(2));
          BOOST_TEST_EQ(y.key_eq(), key_equal(1));
        }

        BOOST_TEST(x.empty());

        BOOST_TEST_EQ(x.hash_function(), hasher(2));
        BOOST_TEST_EQ(x.key_eq(), key_equal(1));
        BOOST_TEST(x.get_allocator() != y.get_allocator());
      });

      BOOST_TEST_EQ(num_transfers, 1u);

      BOOST_TEST_EQ(
        raii::destructor, 2 * 2 * old_size + num_threads * 2 * old_size);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::move_constructor, old_mc + 2 * old_size);
      BOOST_TEST_EQ(raii::copy_constructor,
        old_cc + (num_threads * 2 * reference_map.size()));
    }
    check_raii_counts();

    // lhs non-empty, rhs non-empty, pocma
    {
      raii::reset_counts();

      using pocma_allocator_type =
        pocma_allocator<std::pair<const raii, raii> >;

      using pocma_map_type = boost::unordered::concurrent_flat_map<raii, raii,
        hasher, key_equal, pocma_allocator_type>;

      pocma_map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), pocma_allocator_type(3));

      auto const old_size = x.size();
      auto const old_cc = +raii::copy_constructor;
      auto const old_mc = +raii::move_constructor;

      std::atomic<unsigned> num_transfers{0};

      thread_runner(values, [&x, &values, &num_transfers, &reference_map](
                              boost::span<map_value_type> s) {
        (void)s;

        pocma_map_type y(values.begin(), values.end(), values.size(), hasher(2),
          key_equal(1), pocma_allocator_type(13));

        BOOST_TEST(!y.empty());
        BOOST_TEST(x.get_allocator() != y.get_allocator());

        y = std::move(x);
        if (y.hash_function() == hasher(1)) {
          ++num_transfers;
          test_matches_reference(y, reference_map);

          BOOST_TEST_EQ(y.key_eq(), key_equal(2));
        } else {
          BOOST_TEST_EQ(y.hash_function(), hasher(2));
          BOOST_TEST_EQ(y.key_eq(), key_equal(1));
        }

        BOOST_TEST(x.empty());

        BOOST_TEST_EQ(x.hash_function(), hasher(2));
        BOOST_TEST_EQ(x.key_eq(), key_equal(1));
        BOOST_TEST(x.get_allocator() == y.get_allocator());
      });

      BOOST_TEST_EQ(num_transfers, 1u);

      BOOST_TEST_EQ(
        raii::destructor, 2 * old_size + num_threads * 2 * old_size);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::move_constructor, old_mc);
      BOOST_TEST_EQ(raii::copy_constructor,
        old_cc + (num_threads * 2 * reference_map.size()));
    }
    check_raii_counts();

    // self-assign
    {
      raii::reset_counts();

      map_type x(values.begin(), values.end(), values.size(), hasher(1),
        key_equal(2), allocator_type(3));

      auto const old_cc = +raii::copy_constructor;
      auto const old_mc = +raii::move_constructor;

      thread_runner(
        values, [&x, &reference_map](boost::span<map_value_type> s) {
          (void)s;

          x = std::move(x);

          BOOST_TEST(!x.empty());

          BOOST_TEST_EQ(x.hash_function(), hasher(1));
          BOOST_TEST_EQ(x.key_eq(), key_equal(2));
          BOOST_TEST(x.get_allocator() == allocator_type(3));

          test_matches_reference(x, reference_map);
        });

      BOOST_TEST_EQ(raii::destructor, 0);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
      BOOST_TEST_EQ(raii::move_constructor, old_mc);
      BOOST_TEST_EQ(raii::copy_constructor, old_cc);
    }
    check_raii_counts();
  }
} // namespace

// clang-format off
UNORDERED_TEST(
  copy_assign,
  ((value_type_generator))
  ((default_generator)(sequential)(limited_range)))

UNORDERED_TEST(
  move_assign,
  ((value_type_generator))
  ((default_generator)(sequential)(limited_range)))
// clang-format on

RUN_TESTS()
