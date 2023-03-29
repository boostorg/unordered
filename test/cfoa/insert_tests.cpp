// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../helpers/generators.hpp"
#include "../helpers/test.hpp"

#include <boost/unordered/concurrent_flat_map.hpp>

#include <boost/container_hash/hash.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/core/span.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <vector>

constexpr std::size_t const num_threads = 16;

struct transparent_hash
{
  using is_transparent = void;

  template <class T> std::size_t operator()(T const& t) const noexcept
  {
    return boost::hash<T>()(t);
  }
};

struct transparent_key_equal
{
  using is_transparent = void;

  template <class T, class U> bool operator()(T const& lhs, U const& rhs) const
  {
    return lhs == rhs;
  }
};

struct raii
{
  static std::atomic<std::uint32_t> default_constructor;
  static std::atomic<std::uint32_t> copy_constructor;
  static std::atomic<std::uint32_t> move_constructor;
  static std::atomic<std::uint32_t> destructor;

  static std::atomic<std::uint32_t> copy_assignment;
  static std::atomic<std::uint32_t> move_assignment;

  int x_ = -1;

  raii() { ++default_constructor; }
  raii(int const x) : x_{x} { ++default_constructor; }
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

  friend bool operator==(raii const& lhs, int const x) { return lhs.x_ == x; }
  friend bool operator!=(raii const& lhs, int const x)
  {
    return !(lhs.x_ == x);
  }

  friend bool operator==(int const x, raii const& rhs) { return rhs.x_ == x; }

  friend bool operator!=(int const x, raii const& rhs)
  {
    return !(rhs.x_ == x);
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

std::atomic<std::uint32_t> raii::default_constructor{0};
std::atomic<std::uint32_t> raii::copy_constructor{0};
std::atomic<std::uint32_t> raii::move_constructor{0};
std::atomic<std::uint32_t> raii::destructor{0};
std::atomic<std::uint32_t> raii::copy_assignment{0};
std::atomic<std::uint32_t> raii::move_assignment{0};

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
    boost::span<T> s, std::size_t const nt /* num threads*/)
  {
    std::vector<boost::span<T> > subslices;
    subslices.reserve(nt);

    auto a = s.size() / nt;
    auto b = a;
    if (s.size() % nt != 0) {
      ++b;
    }

    auto num_a = nt;
    auto num_b = std::size_t{0};

    if (nt * b > s.size()) {
      num_a = nt * b - s.size();
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

  template <class T, class F> void thread_runner(std::vector<T>& values, F f)
  {
    std::vector<std::thread> threads;
    auto subslices = split<T>(values, num_threads);

    for (std::size_t i = 0; i < num_threads; ++i) {
      threads.emplace_back([&f, &subslices, i] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto s = subslices[i];
        f(s);
      });
    }
    for (auto& t : threads) {
      t.join();
    }
  }

  struct lvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_inserts{0};
      thread_runner(values, [&x, &num_inserts](boost::span<T> s) {
        for (auto const& r : s) {
          bool b = x.insert(r);
          if (b) {
            ++num_inserts;
          }
        }
      });
      BOOST_TEST_EQ(num_inserts, x.size());
      BOOST_TEST_EQ(raii::copy_assignment, 0);
      BOOST_TEST_EQ(raii::move_assignment, 0);
    }
  } lvalue_inserter;

  struct norehash_lvalue_inserter_type : public lvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      x.reserve(values.size());
      lvalue_inserter_type::operator()(values, x);
      BOOST_TEST_EQ(raii::copy_constructor, 2 * x.size());
      BOOST_TEST_EQ(raii::move_constructor, 0);
    }
  } norehash_lvalue_inserter;

  struct rvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      BOOST_TEST_EQ(raii::copy_constructor, 0);

      std::atomic<std::uint64_t> num_inserts{0};
      thread_runner(values, [&x, &num_inserts](boost::span<T> s) {
        for (auto& r : s) {
          bool b = x.insert(std::move(r));
          if (b) {
            ++num_inserts;
          }
        }
      });
      BOOST_TEST_EQ(num_inserts, x.size());

      if (std::is_same<T, typename X::value_type>::value) {
        BOOST_TEST_EQ(raii::copy_constructor, x.size());
      } else {
        BOOST_TEST_EQ(raii::copy_constructor, 0);
      }

      BOOST_TEST_EQ(raii::copy_assignment, 0);
      BOOST_TEST_EQ(raii::move_assignment, 0);
    }
  } rvalue_inserter;

  struct norehash_rvalue_inserter_type : public rvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      x.reserve(values.size());

      BOOST_TEST_EQ(raii::copy_constructor, 0);
      BOOST_TEST_EQ(raii::move_constructor, 0);

      rvalue_inserter_type::operator()(values, x);

      if (std::is_same<T, typename X::value_type>::value) {
        BOOST_TEST_EQ(raii::copy_constructor, x.size());
        BOOST_TEST_EQ(raii::move_constructor, x.size());
      } else {
        BOOST_TEST_EQ(raii::copy_constructor, 0);
        BOOST_TEST_EQ(raii::move_constructor, 2 * x.size());
      }
    }
  } norehash_rvalue_inserter;

  struct iterator_range_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      thread_runner(
        values, [&x](boost::span<T> s) { x.insert(s.begin(), s.end()); });

      BOOST_TEST_EQ(raii::copy_assignment, 0);
      BOOST_TEST_EQ(raii::move_assignment, 0);
    }
  } iterator_range_inserter;

  struct lvalue_insert_or_assign_copy_assign_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      thread_runner(values, [&x](boost::span<T> s) {
        for (auto& r : s) {
          x.insert_or_assign(r.first, r.second);
        }
      });

      BOOST_TEST_EQ(raii::default_constructor, 0);
      BOOST_TEST_EQ(raii::copy_constructor, 2 * x.size());
      // don't check move construction count here because of rehashing
      BOOST_TEST_GT(raii::move_constructor, 0u);
      BOOST_TEST_EQ(raii::copy_assignment, values.size() - x.size());
      BOOST_TEST_EQ(raii::move_assignment, 0);
    }
  } lvalue_insert_or_assign_copy_assign;

  struct lvalue_insert_or_assign_move_assign_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      thread_runner(values, [&x](boost::span<T> s) {
        for (auto& r : s) {
          x.insert_or_assign(r.first, std::move(r.second));
        }
      });

      BOOST_TEST_EQ(raii::default_constructor, 0);
      BOOST_TEST_EQ(raii::copy_constructor, x.size());
      BOOST_TEST_GT(raii::move_constructor, x.size()); // rehashing
      BOOST_TEST_EQ(raii::copy_assignment, 0);
      BOOST_TEST_EQ(raii::move_assignment, values.size() - x.size());
    }
  } lvalue_insert_or_assign_move_assign;

  struct rvalue_insert_or_assign_copy_assign_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      thread_runner(values, [&x](boost::span<T> s) {
        for (auto& r : s) {
          x.insert_or_assign(std::move(r.first), r.second);
        }
      });

      BOOST_TEST_EQ(raii::default_constructor, 0);
      BOOST_TEST_EQ(raii::copy_constructor, x.size());
      BOOST_TEST_GT(raii::move_constructor, x.size()); // rehashing
      BOOST_TEST_EQ(raii::copy_assignment, values.size() - x.size());
      BOOST_TEST_EQ(raii::move_assignment, 0);
    }
  } rvalue_insert_or_assign_copy_assign;

  struct rvalue_insert_or_assign_move_assign_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      thread_runner(values, [&x](boost::span<T> s) {
        for (auto& r : s) {
          x.insert_or_assign(std::move(r.first), std::move(r.second));
        }
      });

      BOOST_TEST_EQ(raii::default_constructor, 0);
      BOOST_TEST_EQ(raii::copy_constructor, 0);
      BOOST_TEST_GE(raii::move_constructor, 2 * x.size());
      BOOST_TEST_EQ(raii::copy_assignment, 0);
      BOOST_TEST_EQ(raii::move_assignment, values.size() - x.size());
    }
  } rvalue_insert_or_assign_move_assign;

  struct transparent_insert_or_assign_copy_assign_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      using is_transparent =
        typename boost::make_void<typename X::hasher::is_transparent,
          typename X::key_equal::is_transparent>::type;

      boost::ignore_unused<is_transparent>();

      BOOST_TEST_EQ(raii::default_constructor, 0);

      thread_runner(values, [&x](boost::span<T> s) {
        for (auto& r : s) {
          x.insert_or_assign(r.first.x_, r.second);
        }
      });

      BOOST_TEST_EQ(raii::default_constructor, x.size());
      BOOST_TEST_EQ(raii::copy_constructor, x.size());
      BOOST_TEST_GT(raii::move_constructor, x.size()); // rehashing
      BOOST_TEST_EQ(raii::copy_assignment, values.size() - x.size());
      BOOST_TEST_EQ(raii::move_assignment, 0);
    }
  } transparent_insert_or_assign_copy_assign;

  struct transparent_insert_or_assign_move_assign_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      using is_transparent =
        typename boost::make_void<typename X::hasher::is_transparent,
          typename X::key_equal::is_transparent>::type;

      boost::ignore_unused<is_transparent>();

      thread_runner(values, [&x](boost::span<T> s) {
        for (auto& r : s) {
          x.insert_or_assign(r.first.x_, std::move(r.second));
        }
      });

      BOOST_TEST_EQ(raii::default_constructor, x.size());
      BOOST_TEST_EQ(raii::copy_constructor, 0);
      BOOST_TEST_GT(raii::move_constructor, 2 * x.size()); // rehashing
      BOOST_TEST_EQ(raii::copy_assignment, 0);
      BOOST_TEST_EQ(raii::move_assignment, values.size() - x.size());
    }
  } transparent_insert_or_assign_move_assign;

  struct lvalue_insert_or_visit_const_visitor_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_inserts{0};
      std::atomic<std::uint64_t> num_invokes{0};
      thread_runner(values, [&x, &num_inserts, &num_invokes](boost::span<T> s) {
        for (auto& r : s) {
          bool b = x.insert_or_visit(
            r, [&num_invokes](typename X::value_type const& v) {
              (void)v;
              ++num_invokes;
            });

          if (b) {
            ++num_inserts;
          }
        }
      });

      BOOST_TEST_EQ(num_inserts, x.size());
      BOOST_TEST_EQ(num_invokes, values.size() - x.size());

      BOOST_TEST_EQ(raii::default_constructor, 0);
      BOOST_TEST_EQ(raii::copy_constructor, 2 * x.size());
      // don't check move construction count here because of rehashing
      BOOST_TEST_GT(raii::move_constructor, 0);
      BOOST_TEST_EQ(raii::move_assignment, 0);
    }
  } lvalue_insert_or_visit_const_visitor;

  struct lvalue_insert_or_visit_mut_visitor_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_inserts{0};
      std::atomic<std::uint64_t> num_invokes{0};
      thread_runner(values, [&x, &num_inserts, &num_invokes](boost::span<T> s) {
        for (auto& r : s) {
          bool b =
            x.insert_or_visit(r, [&num_invokes](typename X::value_type& v) {
              (void)v;
              ++num_invokes;
            });

          if (b) {
            ++num_inserts;
          }
        }
      });

      BOOST_TEST_EQ(num_inserts, x.size());
      BOOST_TEST_EQ(num_invokes, values.size() - x.size());

      BOOST_TEST_EQ(raii::default_constructor, 0);
      BOOST_TEST_EQ(raii::copy_constructor, 2 * x.size());
      // don't check move construction count here because of rehashing
      BOOST_TEST_GT(raii::move_constructor, 0);
      BOOST_TEST_EQ(raii::move_assignment, 0);
    }
  } lvalue_insert_or_visit_mut_visitor;

  struct rvalue_insert_or_visit_const_visitor_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_inserts{0};
      std::atomic<std::uint64_t> num_invokes{0};
      thread_runner(values, [&x, &num_inserts, &num_invokes](boost::span<T> s) {
        for (auto& r : s) {
          bool b = x.insert_or_visit(
            std::move(r), [&num_invokes](typename X::value_type const& v) {
              (void)v;
              ++num_invokes;
            });

          if (b) {
            ++num_inserts;
          }
        }
      });

      BOOST_TEST_EQ(num_inserts, x.size());
      BOOST_TEST_EQ(num_invokes, values.size() - x.size());

      BOOST_TEST_EQ(raii::default_constructor, 0);

      if (std::is_same<T, typename X::value_type>::value) {
        BOOST_TEST_EQ(raii::copy_constructor, x.size());
        BOOST_TEST_GE(raii::move_constructor, x.size());
      } else {
        BOOST_TEST_EQ(raii::copy_constructor, 0);
        BOOST_TEST_GE(raii::move_constructor, 2 * x.size());
      }
    }
  } rvalue_insert_or_visit_const_visitor;

  struct rvalue_insert_or_visit_mut_visitor_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_inserts{0};
      std::atomic<std::uint64_t> num_invokes{0};
      thread_runner(values, [&x, &num_inserts, &num_invokes](boost::span<T> s) {
        for (auto& r : s) {
          bool b = x.insert_or_visit(
            std::move(r), [&num_invokes](typename X::value_type& v) {
              (void)v;
              ++num_invokes;
            });

          if (b) {
            ++num_inserts;
          }
        }
      });

      BOOST_TEST_EQ(num_inserts, x.size());
      BOOST_TEST_EQ(num_invokes, values.size() - x.size());

      BOOST_TEST_EQ(raii::default_constructor, 0);
      if (std::is_same<T, typename X::value_type>::value) {
        BOOST_TEST_EQ(raii::copy_constructor, x.size());
        BOOST_TEST_GE(raii::move_constructor, x.size());
      } else {
        BOOST_TEST_EQ(raii::copy_constructor, 0);
        BOOST_TEST_GE(raii::move_constructor, 2 * x.size());
      }
    }
  } rvalue_insert_or_visit_mut_visitor;

  struct iterator_range_insert_or_visit_const_visitor_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_invokes{0};
      thread_runner(values, [&x, &num_invokes](boost::span<T> s) {
        x.insert_or_visit(
          s.begin(), s.end(), [&num_invokes](typename X::value_type const& v) {
            (void)v;
            ++num_invokes;
          });
      });

      BOOST_TEST_EQ(num_invokes, values.size() - x.size());

      BOOST_TEST_EQ(raii::default_constructor, 0);
      BOOST_TEST_EQ(raii::copy_constructor, 2 * x.size());
      BOOST_TEST_GT(raii::move_constructor, 0);
    }
  } iterator_range_insert_or_visit_const_visitor;

  struct iterator_range_insert_or_visit_mut_visitor_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_invokes{0};
      thread_runner(values, [&x, &num_invokes](boost::span<T> s) {
        x.insert_or_visit(
          s.begin(), s.end(), [&num_invokes](typename X::value_type const& v) {
            (void)v;
            ++num_invokes;
          });
      });

      BOOST_TEST_EQ(num_invokes, values.size() - x.size());

      BOOST_TEST_EQ(raii::default_constructor, 0);
      BOOST_TEST_EQ(raii::copy_constructor, 2 * x.size());
      BOOST_TEST_GT(raii::move_constructor, 0);
    }
  } iterator_range_insert_or_visit_mut_visitor;

  template <class X, class G, class F>
  void insert(X*, G gen, F inserter, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
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
        if (rg == test::sequential) {
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
  }

  template <class X> void insert_initializer_list(X*)
  {
    using value_type = typename X::value_type;

    std::initializer_list<value_type> values{
      value_type{raii{0}, raii{0}},
      value_type{raii{1}, raii{1}},
      value_type{raii{2}, raii{2}},
      value_type{raii{3}, raii{3}},
      value_type{raii{4}, raii{4}},
      value_type{raii{5}, raii{5}},
      value_type{raii{6}, raii{6}},
      value_type{raii{6}, raii{6}},
      value_type{raii{7}, raii{7}},
      value_type{raii{8}, raii{8}},
      value_type{raii{9}, raii{9}},
      value_type{raii{10}, raii{10}},
      value_type{raii{9}, raii{9}},
      value_type{raii{8}, raii{8}},
      value_type{raii{7}, raii{7}},
      value_type{raii{6}, raii{6}},
      value_type{raii{5}, raii{5}},
      value_type{raii{4}, raii{4}},
      value_type{raii{3}, raii{3}},
      value_type{raii{2}, raii{2}},
      value_type{raii{1}, raii{1}},
      value_type{raii{0}, raii{0}},
    };

    std::vector<raii> dummy;

    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());
    raii::reset_counts();

    {
      {
        X x;

        thread_runner(
          dummy, [&x, &values](boost::span<raii>) { x.insert(values); });

        BOOST_TEST_EQ(x.size(), reference_map.size());

        BOOST_TEST_EQ(x.size(), x.visit_all([&](value_type const& kv) {
          BOOST_TEST(reference_map.contains(kv.first));
          BOOST_TEST_EQ(kv.second, reference_map[kv.first]);
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

    {
      {
        std::atomic<std::uint64_t> num_invokes{0};

        X x;

        thread_runner(dummy, [&x, &values, &num_invokes](boost::span<raii>) {
          x.insert_or_visit(values, [&num_invokes](typename X::value_type& v) {
            (void)v;
            ++num_invokes;
          });

          x.insert_or_visit(
            values, [&num_invokes](typename X::value_type const& v) {
              (void)v;
              ++num_invokes;
            });
        });

        BOOST_TEST_EQ(num_invokes, (values.size() - x.size()) +
                                     (num_threads - 1) * values.size() +
                                     num_threads * values.size());
        BOOST_TEST_EQ(x.size(), reference_map.size());

        BOOST_TEST_EQ(x.size(), x.visit_all([&](value_type const& kv) {
          BOOST_TEST(reference_map.contains(kv.first));
          BOOST_TEST_EQ(kv.second, reference_map[kv.first]);
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
  }

  boost::unordered::concurrent_flat_map<raii, raii>* map;
  boost::unordered::concurrent_flat_map<raii, raii, transparent_hash,
    transparent_key_equal>* transparent_map;

} // namespace

using test::default_generator;
using test::limited_range;
using test::sequential;

// clang-format off
UNORDERED_TEST(
  insert_initializer_list,
  ((map)))

UNORDERED_TEST(
  insert,
  ((map))
  ((value_type_generator)(init_type_generator))
  ((lvalue_inserter)(rvalue_inserter)(iterator_range_inserter)
   (norehash_lvalue_inserter)(norehash_rvalue_inserter)
   (lvalue_insert_or_visit_const_visitor)(lvalue_insert_or_visit_mut_visitor)
   (rvalue_insert_or_visit_const_visitor)(rvalue_insert_or_visit_mut_visitor)
   (iterator_range_insert_or_visit_const_visitor)(iterator_range_insert_or_visit_mut_visitor))
  ((default_generator)(sequential)(limited_range)))

UNORDERED_TEST(
  insert,
  ((map))
  ((init_type_generator))
  ((lvalue_insert_or_assign_copy_assign)(lvalue_insert_or_assign_move_assign)
   (rvalue_insert_or_assign_copy_assign)(rvalue_insert_or_assign_move_assign))
  ((default_generator)(sequential)(limited_range)))

UNORDERED_TEST(
  insert,
  ((transparent_map))
  ((init_type_generator))
  ((transparent_insert_or_assign_copy_assign)(transparent_insert_or_assign_move_assign))
  ((default_generator)(sequential)(limited_range)))
// clang-format on

RUN_TESTS()
