//
// Copyright 2023 Braden Ganetsky.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../helpers/unordered.hpp"

#include "../helpers/count.hpp"
#include "../helpers/test.hpp"

namespace emplace_smf_tests {
  using test::smf_count;
  using test::smf_counted_object;

  using counted_key = smf_counted_object<struct key_tag_>;
  using counted_value = smf_counted_object<struct value_tag_>;
  void reset_counts()
  {
    counted_key::reset_count();
    counted_value::reset_count();
  }

#ifdef BOOST_UNORDERED_FOA_TESTS
  static boost::unordered_flat_map<counted_key, counted_value>* test_smf_map;
  static boost::unordered_node_map<counted_key, counted_value>*
    test_smf_node_map;
  static boost::unordered_flat_set<counted_value>* test_smf_set;
  static boost::unordered_node_set<counted_value>* test_smf_node_set;
#define EMPLACE_SMF_TESTS_MAP_ARGS ((test_smf_map)(test_smf_node_map))
#define EMPLACE_SMF_TESTS_SET_ARGS ((test_smf_set)(test_smf_node_set))
#else
  static boost::unordered_map<counted_key, counted_value>* test_smf_map;
  static boost::unordered_set<counted_value>* test_smf_set;
#define EMPLACE_SMF_TESTS_MAP_ARGS ((test_smf_map))
#define EMPLACE_SMF_TESTS_SET_ARGS ((test_smf_set))
#endif

  template <class X> static void emplace_smf_value_type_map(X*)
  {
    using container = X;
    using value_type = typename container::value_type;

    container x;

    {
      value_type val{counted_key{}, counted_value{}};
      x.clear();
      reset_counts();
      x.emplace(val);
      BOOST_TEST_EQ(counted_key::count, (smf_count{0, 1, 0, 0, 0, 0}));
      BOOST_TEST_EQ(counted_value::count, (smf_count{0, 1, 0, 0, 0, 0}));
    }

    {
      value_type val{counted_key{}, counted_value{}};
      x.clear();
      reset_counts();
      x.emplace(std::move(val));
      BOOST_TEST_EQ(counted_key::count, (smf_count{0, 1, 0, 0, 0, 0}));
      BOOST_TEST_EQ(counted_value::count, (smf_count{0, 0, 1, 0, 0, 0}));
    }

    {
      x.clear();
      reset_counts();
      x.emplace(value_type{counted_key{}, counted_value{}});
      BOOST_TEST_EQ(counted_key::count, (smf_count{1, 1, 1, 0, 0, 2}));
      BOOST_TEST_EQ(counted_value::count, (smf_count{1, 0, 2, 0, 0, 2}));
    }

    {
      counted_key key{};
      counted_value value{};
      x.clear();
      reset_counts();
      x.emplace(value_type{std::move(key), std::move(value)});
      BOOST_TEST_EQ(counted_key::count, (smf_count{0, 1, 1, 0, 0, 1}));
      BOOST_TEST_EQ(counted_value::count, (smf_count{0, 0, 2, 0, 0, 1}));
    }
  }

  UNORDERED_TEST(emplace_smf_value_type_map, EMPLACE_SMF_TESTS_MAP_ARGS)

  template <class X> static void emplace_smf_init_type_map(X*)
  {
    using container = X;
#ifdef BOOST_UNORDERED_FOA_TESTS
    using init_type = typename container::init_type;
#else
    using raw_key =
      typename std::remove_const<typename container::key_type>::type;
    using init_type = std::pair<raw_key, typename container::mapped_type>;
#endif

    container x;

    {
      init_type val{counted_key{}, counted_value{}};
      x.clear();
      reset_counts();
      x.emplace(val);
      BOOST_TEST_EQ(counted_key::count, (smf_count{0, 1, 0, 0, 0, 0}));
      BOOST_TEST_EQ(counted_value::count, (smf_count{0, 1, 0, 0, 0, 0}));
    }

    {
      init_type val{counted_key{}, counted_value{}};
      x.clear();
      reset_counts();
      x.emplace(std::move(val));
      BOOST_TEST_EQ(counted_key::count, (smf_count{0, 0, 1, 0, 0, 0}));
      BOOST_TEST_EQ(counted_value::count, (smf_count{0, 0, 1, 0, 0, 0}));
    }

    {
      x.clear();
      reset_counts();
      x.emplace(init_type{counted_key{}, counted_value{}});
      BOOST_TEST_EQ(counted_key::count, (smf_count{1, 0, 2, 0, 0, 2}));
      BOOST_TEST_EQ(counted_value::count, (smf_count{1, 0, 2, 0, 0, 2}));
    }

    {
      counted_key key{};
      counted_value value{};
      x.clear();
      reset_counts();
      x.emplace(init_type{std::move(key), std::move(value)});
      BOOST_TEST_EQ(counted_key::count, (smf_count{0, 0, 2, 0, 0, 1}));
      BOOST_TEST_EQ(counted_value::count, (smf_count{0, 0, 2, 0, 0, 1}));
    }
  }

  UNORDERED_TEST(emplace_smf_init_type_map, EMPLACE_SMF_TESTS_MAP_ARGS)

  template <class X> static void emplace_smf_value_type_set(X*)
  {
    using container = X;
    using value_type = typename container::value_type;
#ifdef BOOST_UNORDERED_FOA_TESTS
    BOOST_STATIC_ASSERT(
      std::is_same<value_type, typename container::init_type>::value);
#endif
    BOOST_STATIC_ASSERT(std::is_same<value_type, counted_value>::value);

    container x;

    {
      counted_value val{};
      x.clear();
      reset_counts();
      x.emplace(val);
      BOOST_TEST_EQ(counted_value::count, (smf_count{0, 1, 0, 0, 0, 0}));
    }

    {
      counted_value val{};
      x.clear();
      reset_counts();
      x.emplace(std::move(val));
      BOOST_TEST_EQ(counted_value::count, (smf_count{0, 0, 1, 0, 0, 0}));
    }

    {
      x.clear();
      reset_counts();
      x.emplace(counted_value{});
      BOOST_TEST_EQ(counted_value::count, (smf_count{1, 0, 1, 0, 0, 1}));
    }
  }

  UNORDERED_TEST(emplace_smf_value_type_set, EMPLACE_SMF_TESTS_SET_ARGS)
} // namespace emplace_smf_tests

RUN_TESTS()
