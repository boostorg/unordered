// Copyright 2021 Christian Mazakas.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off
#include "../helpers/prefix.hpp"
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "../helpers/postfix.hpp"
// clang-format on

#include "../helpers/test.hpp"

#include <boost/container_hash/hash.hpp>

struct key
{
  int x_;
  static int count_;

  key(int x) : x_(x) { ++count_; }
  key(key const& k) : x_(k.x_) { ++count_; }
};

int key::count_;

struct transparent_hasher
{
  typedef void is_transparent;

  std::size_t operator()(key const& k) const
  {
    return boost::hash<int>()(k.x_);
  }

  std::size_t operator()(int const k) const { return boost::hash<int>()(k); }
};

struct transparent_key_equal
{
  typedef void is_transparent;

  static bool was_called_;

  bool operator()(key const& k1, key const& k2) const { return k1.x_ == k2.x_; }
  bool operator()(int const x, key const& k1) const
  {
    was_called_ = true;
    return k1.x_ == x;
  }
};

bool transparent_key_equal::was_called_;

struct hasher
{
  std::size_t operator()(key const& k) const
  {
    return boost::hash<int>()(k.x_);
  }
};

struct key_equal
{
  static bool was_called_;

  bool operator()(key const& k1, key const& k2) const { return k1.x_ == k2.x_; }
  bool operator()(int const x, key const& k1) const
  {
    was_called_ = true;
    return k1.x_ == x;
  }
};

bool key_equal::was_called_;

void count_reset()
{
  key::count_ = 0;
  transparent_key_equal::was_called_ = false;
  key_equal::was_called_ = false;
}

template <class UnorderedMap> void test_non_transparent_count()
{
  count_reset();

  UnorderedMap map;

  // initial `key(0)` expression increases the count
  // then copying into the `unordered_map` increments the count again thus we
  // have 2
  //
  map[key(0)] = 1337;
  BOOST_TEST(key::count_ == 2);

  // rely on the implicit constructor here to spawn a new object which
  // increases the count
  //
  std::size_t count = 0;
  count = map.count(0);

  BOOST_TEST(count == 1);
  BOOST_TEST(key::count_ == 3);
  BOOST_TEST(!map.key_eq().was_called_);

  count = map.count(1);

  BOOST_TEST(count == 0);
  BOOST_TEST(key::count_ == 4);

  count = map.count(key(0));
  BOOST_TEST(count == 1);
  BOOST_TEST(key::count_ == 5);
}

UNORDERED_AUTO_TEST (unordered_map_transparent_count) {
  {
    // transparent Hash, transparent KeyEqual
    //
    count_reset();

    boost::unordered_map<key, int, transparent_hasher, transparent_key_equal>
      map;

    // initial `key(0)` expression increases the count
    // then copying into the `unordered_map` increments the count again thus we
    // have 2
    //
    map[key(0)] = 1337;
    BOOST_TEST(key::count_ == 2);

    // now the number of `key` objects created should be a constant and never
    // touched again
    //
    std::size_t count = 0;
    count = map.count(0);

    BOOST_TEST(count == 1);
    BOOST_TEST(key::count_ == 2);
    BOOST_TEST(map.key_eq().was_called_);

    count = map.count(1);

    BOOST_TEST(count == 0);
    BOOST_TEST(key::count_ == 2);

    count = map.count(key(0));
    BOOST_TEST(count == 1);
    BOOST_TEST(key::count_ == 3);
  }

  // non-transparent Hash, non-transparent KeyEqual
  //
  test_non_transparent_count<
    boost::unordered_map<key, int, hasher, key_equal> >();

  // transparent Hash, non-transparent KeyEqual
  //
  test_non_transparent_count<
    boost::unordered_map<key, int, transparent_hasher, key_equal> >();

  // non-transparent Hash, transparent KeyEqual
  //
  test_non_transparent_count<
    boost::unordered_map<key, int, hasher, transparent_key_equal> >();
}

RUN_TESTS()
