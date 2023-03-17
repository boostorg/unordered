// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../helpers/generators.hpp"
#include "../helpers/test.hpp"

#include <boost/unordered/concurrent_flat_map.hpp>

#include <boost/container_hash/hash.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

#include <atomic>
#include <iostream>
#include <vector>

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

  static void reset_counts()
  {
    default_constructor = 0;
    copy_constructor = 0;
    move_constructor = 0;
    destructor = 0;
    copy_assignment = 0;
    move_assignment = 0;
  }

  friend std::ostream& operator<<(std::ostream& os, raii const& rhs)
  {
    os << "{ x_: " << rhs.x_ << "}";
    return os;
  }

  friend std::ostream& operator<<(
    std::ostream& os, std::pair<raii const, raii> const& rhs)
  {
    os << "pair<" << rhs.first << ", " << rhs.second << ">";
    return os;
  }

  friend bool operator==(raii const& lhs, raii const& rhs)
  {
    return lhs.x_ == rhs.x_;
  }

  friend bool operator!=(raii const& lhs, raii const& rhs)
  {
    return !(lhs == rhs);
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

std::vector<std::pair<raii const, raii> > make_random_values(
  std::size_t count, test::random_generator rg)
{
  std::vector<std::pair<raii const, raii> > v;
  v.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    int* p = nullptr;
    int a = generate(p, rg);
    int b = generate(p, rg);
    v.emplace_back(raii{a}, raii{b});
  }
  return v;
}

namespace {
  test::seed_t initialize_seed(78937);

  struct lvalue_inserter_type
  {
    template <class T, class X>
    void operator()(std::vector<T> const& values, X& x)
    {
      for (auto const& r : values) {
        bool b = x.insert(r);
        (void)b;
      }
    }
  } lvalue_inserter;

  struct rvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      for (auto& r : values) {
        bool b = x.insert(std::move(r));
        (void)b;
      }
    }
  } rvalue_inserter;

  template <class X, class F>
  void insert(X*, F inserter, test::random_generator generator)
  {
    auto values = make_random_values(1024, generator);
    BOOST_TEST_GT(values.size(), 0);

    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());

    raii::reset_counts();

    {
      X x;

      inserter(values, x);

      BOOST_TEST_EQ(x.size(), reference_map.size());

      using value_type = typename X::value_type;
      BOOST_TEST_EQ(x.size(), x.visit_all([&](value_type const& kv) {
        BOOST_TEST(reference_map.contains(kv.first));
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
using test::generate_collisions;
using test::limited_range;

// clang-format off
UNORDERED_TEST(
  insert, ((map))
          ((lvalue_inserter)(rvalue_inserter))
          ((default_generator)(generate_collisions)(limited_range)))
// clang-format on

RUN_TESTS()
