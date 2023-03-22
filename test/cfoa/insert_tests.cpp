// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../helpers/generators.hpp"
#include "../helpers/test.hpp"

#include <boost/unordered/concurrent_flat_map.hpp>

#include <boost/container_hash/hash.hpp>
#include <boost/core/span.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <vector>

constexpr std::size_t const num_threads = 16;

struct raii
{
  static std::atomic_int default_constructor;
  static std::atomic_int copy_constructor;
  static std::atomic_int move_constructor;
  static std::atomic_int destructor;

  static std::atomic_int copy_assignment;
  static std::atomic_int move_assignment;

  int x_ = -1;

  raii() { ++default_constructor; }
  explicit raii(int const x) : x_{x} { ++default_constructor; }
  raii(raii const& rhs) : x_{rhs.x_} { ++copy_constructor; }
  raii(raii&& rhs) noexcept : x_{rhs.x_}
  {
    rhs.x_ = -1;
    ++move_constructor;
  }
  ~raii() { ++destructor; }

  raii& operator=(raii const& rhs)
  {
    ++copy_assignment;
    if (this != &rhs) {
      x_ = rhs.x_;
    }
    return *this;
  }

  raii& operator=(raii&& rhs) noexcept
  {
    ++move_assignment;
    if (this != &rhs) {
      x_ = rhs.x_;
      rhs.x_ = -1;
    }
    return *this;
  }

  friend bool operator==(raii const& lhs, raii const& rhs)
  {
    return lhs.x_ == rhs.x_;
  }

  friend bool operator!=(raii const& lhs, raii const& rhs)
  {
    return !(lhs == rhs);
  }

  friend std::ostream& operator<<(std::ostream& os, raii const& rhs)
  {
    os << "{ x_: " << rhs.x_ << " }";
    return os;
  }

  friend std::ostream& operator<<(
    std::ostream& os, std::pair<raii const, raii> const& rhs)
  {
    os << "pair<" << rhs.first << ", " << rhs.second << ">";
    return os;
  }

  static void reset_counts()
  {
    default_constructor = 0;
    copy_constructor = 0;
    move_constructor = 0;
    destructor = 0;
    copy_assignment = 0;
    move_assignment = 0;
  }
};

std::atomic_int raii::default_constructor{0};
std::atomic_int raii::copy_constructor{0};
std::atomic_int raii::move_constructor{0};
std::atomic_int raii::destructor{0};
std::atomic_int raii::copy_assignment{0};
std::atomic_int raii::move_assignment{0};

std::size_t hash_value(raii const& r) noexcept
{
  boost::hash<int> hasher;
  return hasher(r.x_);
}

template <class F>
auto make_random_values(std::size_t count, F f) -> std::vector<decltype(f())>
{
  using vector_type = std::vector<decltype(f())>;

  vector_type v;
  v.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    v.emplace_back(f());
  }
  return v;
}

namespace {
  test::seed_t initialize_seed(78937);

  struct value_type_generator_type
  {
    std::pair<raii const, raii> operator()(test::random_generator rg)
    {
      int* p = nullptr;
      int a = generate(p, rg);
      int b = generate(p, rg);
      return std::make_pair(raii{a}, raii{b});
    }
  } value_type_generator;

  struct init_type_generator_type
  {
    std::pair<raii, raii> operator()(test::random_generator rg)
    {
      int* p = nullptr;
      int a = generate(p, rg);
      int b = generate(p, rg);
      return std::make_pair(raii{a}, raii{b});
    }
  } init_type_generator;

  template <class T>
  std::vector<boost::span<T> > split(
    std::vector<T>& vec, std::size_t const nt /* num threads*/)
  {
    std::vector<boost::span<T> > subslices;
    subslices.reserve(nt);

    boost::span<T> s(vec);

    auto a = vec.size() / nt;
    auto b = a;
    if (vec.size() % nt != 0) {
      ++b;
    }

    auto num_a = nt;
    auto num_b = std::size_t{0};

    if (nt * b > vec.size()) {
      num_a = nt * b - vec.size();
      num_b = nt - num_a;
    }

    auto sub_b = s.subspan(0, num_b * b);
    auto sub_a = s.subspan(num_b * b);

    for (std::size_t i = 0; i < num_b; ++i) {
      subslices.push_back(sub_b.subspan(i * b, b));
    }

    for (std::size_t i = 0; i < num_a; ++i) {
      auto const is_last = i == (num_a - 1);
      subslices.push_back(
        sub_a.subspan(i * a, is_last ? boost::dynamic_extent : a));
    }

    return subslices;
  }

  struct lvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::vector<std::thread> threads;
      auto subslices = split(values, num_threads);

      for (std::size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&x, &subslices, i] {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));

          {
            auto s = subslices[i];
            for (auto const& r : s) {
              bool b = x.insert(r);
              (void)b;
            }
          }
        });
      }
      for (auto& t : threads) {
        t.join();
      }
    }
  } lvalue_inserter;

  struct rvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      BOOST_TEST_EQ(raii::copy_constructor, 0);

      std::vector<std::thread> threads;
      auto subslices = split(values, num_threads);

      for (std::size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&x, &subslices, i] {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));

          {
            auto s = subslices[i];
            for (auto& r : s) {
              bool b = x.insert(std::move(r));
              (void)b;
            }
          }
        });
      }
      for (auto& t : threads) {
        t.join();
      }

      if (std::is_same<T, typename X::value_type>::value) {
        BOOST_TEST_EQ(raii::copy_constructor, x.size());
      } else {
        BOOST_TEST_EQ(raii::copy_constructor, 0);
      }
    }
  } rvalue_inserter;

  struct iterator_range_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::vector<std::thread> threads;
      auto subslices = split(values, num_threads);

      for (std::size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&x, &subslices, i] {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));

          {
            auto s = subslices[i];
            x.insert(s.begin(), s.end());
          }
        });
      }
      for (auto& t : threads) {
        t.join();
      }
    }
  } iterator_range_inserter;

  template <class X, class G, class F>
  void insert(X*, G gen, F inserter, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
    BOOST_TEST_GT(values.size(), 0u);

    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());

    raii::reset_counts();

    {
      X x;

      inserter(values, x);

      BOOST_TEST_EQ(x.size(), reference_map.size());

      using value_type = typename X::value_type;
      BOOST_TEST_EQ(x.size(), x.visit_all([&](value_type const& kv) {
        if (BOOST_TEST(reference_map.contains(kv.first)) &&
            rg == test::sequential) {
          BOOST_TEST_EQ(kv.second, reference_map[kv.first]);
        }
      }));
    }

    BOOST_TEST_GE(raii::default_constructor, 0);
    BOOST_TEST_GE(raii::copy_constructor, 0);
    BOOST_TEST_GE(raii::move_constructor, 0);
    BOOST_TEST_GT(raii::destructor, 0);

    BOOST_TEST_EQ(raii::default_constructor + raii::copy_constructor +
                    raii::move_constructor,
      raii::destructor);

    BOOST_TEST_EQ(raii::copy_assignment, 0);
    BOOST_TEST_EQ(raii::move_assignment, 0);
  }

  boost::unordered::concurrent_flat_map<raii, raii>* map;

} // namespace

using test::default_generator;
using test::limited_range;
using test::sequential;

// clang-format off
UNORDERED_TEST(
  insert, ((map))
          ((value_type_generator)(init_type_generator))
          ((lvalue_inserter)(rvalue_inserter)(iterator_range_inserter))
          ((default_generator)(sequential)(limited_range)))
// clang-format on

RUN_TESTS()
