// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "helpers.hpp"

#include <boost/unordered/concurrent_flat_map.hpp>

#include <boost/core/ignore_unused.hpp>

#include <functional>
#include <vector>

namespace {
  test::seed_t initialize_seed(335740237);

  struct lvalue_visitor_type
  {
    template <class T, class X>
    void operator()(std::vector<T>& values, X& x, test::random_generator rg)
    {
      using value_type = typename X::value_type;

      std::atomic<std::uint64_t> num_visits{0};
      std::atomic<std::uint64_t> total_count{0};

      auto mut_visitor = [&num_visits, rg](int r, int r2) {
        return [&num_visits, r, r2, rg](value_type& v) {
          BOOST_TEST_EQ(v.first.x_, r);
          if (rg == test::sequential) {
            BOOST_TEST_EQ(v.second.x_, r2);
          }
          ++num_visits;
        };
      };

      auto const_visitor = [&num_visits, rg](int r, int r2) {
        return [&num_visits, r, r2, rg](value_type const& v) {
          BOOST_TEST_EQ(v.first.x_, r);
          if (rg == test::sequential) {
            BOOST_TEST_EQ(v.second.x_, r2);
          }
          ++num_visits;
        };
      };

      {
        thread_runner(
          values, [&x, &mut_visitor, &total_count](boost::span<T> s) {
            for (auto const& val : s) {
              auto r = val.first.x_;
              BOOST_ASSERT(r >= 0);
              auto r2 = val.second.x_;

              auto count = x.visit(val.first, mut_visitor(r, r2));

              BOOST_TEST_EQ(count, 1u);
              total_count += count;

              count = x.visit(val.second, mut_visitor(r, r2));
              BOOST_TEST_EQ(count, 0u);
            }
          });

        BOOST_TEST_EQ(num_visits, values.size());
        BOOST_TEST_EQ(total_count, values.size());

        num_visits = 0;
        total_count = 0;
      }

      {
        thread_runner(
          values, [&x, &const_visitor, &total_count](boost::span<T> s) {
            for (auto const& val : s) {
              auto r = val.first.x_;
              BOOST_ASSERT(r >= 0);
              auto r2 = val.second.x_;

              auto const& y = x;
              auto count = y.visit(val.first, const_visitor(r, r2));

              BOOST_TEST_EQ(count, 1u);
              total_count += count;

              count = y.visit(val.second, const_visitor(r, r2));
              BOOST_TEST_EQ(count, 0u);
            }
          });

        BOOST_TEST_EQ(num_visits, values.size());
        BOOST_TEST_EQ(total_count, values.size());

        num_visits = 0;
        total_count = 0;
      }

      {
        thread_runner(
          values, [&x, &const_visitor, &total_count](boost::span<T> s) {
            for (auto const& val : s) {
              auto r = val.first.x_;
              BOOST_ASSERT(r >= 0);
              auto r2 = val.second.x_;

              auto count = x.cvisit(val.first, const_visitor(r, r2));

              BOOST_TEST_EQ(count, 1u);
              total_count += count;

              count = x.cvisit(val.second, const_visitor(r, r2));
              BOOST_TEST_EQ(count, 0u);
            }
          });

        BOOST_TEST_EQ(num_visits, values.size());
        BOOST_TEST_EQ(total_count, values.size());

        num_visits = 0;
        total_count = 0;
      }
    }
  } lvalue_visitor;

  struct transp_visitor_type
  {
    template <class T, class X>
    void operator()(std::vector<T>& values, X& x, test::random_generator rg)
    {
      using value_type = typename X::value_type;

      std::atomic<std::uint64_t> num_visits{0};
      std::atomic<std::uint64_t> total_count{0};

      auto mut_visitor = [&num_visits, rg](int r, int r2) {
        return [&num_visits, r, r2, rg](value_type& v) {
          BOOST_TEST_EQ(v.first.x_, r);
          if (rg == test::sequential) {
            BOOST_TEST_EQ(v.second.x_, r2);
          }
          ++num_visits;
        };
      };

      auto const_visitor = [&num_visits, rg](int r, int r2) {
        return [&num_visits, r, r2, rg](value_type const& v) {
          BOOST_TEST_EQ(v.first.x_, r);
          if (rg == test::sequential) {
            BOOST_TEST_EQ(v.second.x_, r2);
          }
          ++num_visits;
        };
      };

      {
        thread_runner(
          values, [&x, &mut_visitor, &total_count](boost::span<T> s) {
            for (auto const& val : s) {
              auto r = val.first.x_;
              BOOST_ASSERT(r >= 0);
              auto r2 = val.second.x_;

              auto count = x.visit(val.first.x_, mut_visitor(r, r2));

              BOOST_TEST_EQ(count, 1u);
              total_count += count;

              count = x.visit(val.second.x_, mut_visitor(r, r2));
              BOOST_TEST_EQ(count, 0u);
            }
          });

        BOOST_TEST_EQ(num_visits, values.size());
        BOOST_TEST_EQ(total_count, values.size());

        num_visits = 0;
        total_count = 0;
      }

      {
        thread_runner(
          values, [&x, &const_visitor, &total_count](boost::span<T> s) {
            for (auto const& val : s) {
              auto r = val.first.x_;
              BOOST_ASSERT(r >= 0);
              auto r2 = val.second.x_;

              auto const& y = x;
              auto count = y.visit(val.first.x_, const_visitor(r, r2));

              BOOST_TEST_EQ(count, 1u);
              total_count += count;

              count = y.visit(val.second.x_, const_visitor(r, r2));
              BOOST_TEST_EQ(count, 0u);
            }
          });

        BOOST_TEST_EQ(num_visits, values.size());
        BOOST_TEST_EQ(total_count, values.size());

        num_visits = 0;
        total_count = 0;
      }

      {
        thread_runner(
          values, [&x, &const_visitor, &total_count](boost::span<T> s) {
            for (auto const& val : s) {
              auto r = val.first.x_;
              BOOST_ASSERT(r >= 0);
              auto r2 = val.second.x_;

              auto count = x.cvisit(val.first.x_, const_visitor(r, r2));

              BOOST_TEST_EQ(count, 1u);
              total_count += count;

              count = x.cvisit(val.second.x_, const_visitor(r, r2));
              BOOST_TEST_EQ(count, 0u);
            }
          });

        BOOST_TEST_EQ(num_visits, values.size());
        BOOST_TEST_EQ(total_count, values.size());

        num_visits = 0;
        total_count = 0;
      }
    }
  } transp_visitor;

  template <class X, class G, class F>
  void visit(X*, G gen, F visitor, test::random_generator rg)
  {
    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
    for (auto& val : values) {
      if (val.second.x_ == 0) {
        val.second.x_ = 1;
      }
      val.second.x_ *= -1;
    }

    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());
    raii::reset_counts();

    {
      X x;
      for (auto const& v : values) {
        x.insert(v);
      }
      BOOST_TEST_EQ(x.size(), reference_map.size());

      std::uint64_t old_default_constructor = raii::default_constructor;
      std::uint64_t old_copy_constructor = raii::copy_constructor;
      std::uint64_t old_move_constructor = raii::move_constructor;
      std::uint64_t old_copy_assignment = raii::copy_assignment;
      std::uint64_t old_move_assignment = raii::move_assignment;

      visitor(values, x, rg);

      BOOST_TEST_EQ(old_default_constructor, raii::default_constructor);
      BOOST_TEST_EQ(old_copy_constructor, raii::copy_constructor);
      BOOST_TEST_EQ(old_move_constructor, raii::move_constructor);
      BOOST_TEST_EQ(old_copy_assignment, raii::copy_assignment);
      BOOST_TEST_EQ(old_move_assignment, raii::move_assignment);
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
  boost::unordered::concurrent_flat_map<raii, raii, transp_hash,
    transp_key_equal>* transp_map;

} // namespace

using test::default_generator;
using test::limited_range;
using test::sequential;

// clang-format off

UNORDERED_TEST(
  visit,
  ((map))
  ((value_type_generator)(init_type_generator))
  ((lvalue_visitor))
  ((default_generator)(sequential)(limited_range)))

UNORDERED_TEST(
  visit,
  ((transp_map))
  ((value_type_generator)(init_type_generator))
  ((transp_visitor))
  ((default_generator)(sequential)(limited_range)))

// clang-format on

RUN_TESTS()
