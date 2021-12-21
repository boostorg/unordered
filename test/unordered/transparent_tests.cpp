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
  bool operator()(key const& k1, int const x) const
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

template <class UnorderedMap> void test_transparent_count()
{
  count_reset();

  UnorderedMap map;

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

template <class UnorderedMap> void test_transparent_find()
{
  count_reset();

  typedef typename UnorderedMap::const_iterator map_iterator;
  typedef typename UnorderedMap::value_type pair;

  UnorderedMap map;

  int n = 5;

  for (int i = 0; i < n; ++i) {
    map.insert(std::make_pair(i, i));
  }

  int const expected_key_count = key::count_;

  // explicitly test `find()` and `find() const` separately
  //

  {
    UnorderedMap& m = map;

    for (int i = 0; i < n; ++i) {
      map_iterator pos = m.find(i);
      BOOST_TEST(pos != m.end());

      pair const& p = *pos;
      int const v = p.second;

      BOOST_TEST_EQ(v, i);
    }

    BOOST_TEST_EQ(key::count_, expected_key_count);

    map_iterator pos = m.find(1337);
    BOOST_TEST(pos == m.end());
    BOOST_TEST_EQ(key::count_, expected_key_count);
  }

  {
    UnorderedMap const& m = map;

    for (int i = 0; i < n; ++i) {
      map_iterator pos = m.find(i);
      BOOST_TEST(pos != m.end());

      pair const& p = *pos;
      int const v = p.second;

      BOOST_TEST(v == i);
    }

    BOOST_TEST_EQ(key::count_, expected_key_count);

    map_iterator pos = m.find(1337);
    BOOST_TEST(pos == m.end());
    BOOST_TEST_EQ(key::count_, expected_key_count);
  }
}

template <class UnorderedMap> void test_non_transparent_find()
{
  count_reset();

  typedef typename UnorderedMap::const_iterator map_iterator;
  typedef typename UnorderedMap::value_type pair;

  UnorderedMap map;

  int n = 5;

  for (int i = 0; i < n; ++i) {
    map.insert(std::make_pair(i, i));
  }

  int key_count = key::count_;

  // explicitly test `find()` and `find() const` separately
  //

  {
    UnorderedMap& m = map;

    for (int i = 0; i < n; ++i) {
      map_iterator pos = m.find(i);
      BOOST_TEST(pos != m.end());

      pair const& p = *pos;
      int const v = p.second;

      BOOST_TEST_EQ(v, i);
    }
    BOOST_TEST_EQ(key::count_, n + key_count);

    map_iterator pos = m.find(1337);
    BOOST_TEST(pos == m.end());
    BOOST_TEST_EQ(key::count_, 1 + n + key_count);

    key_count = key::count_;
  }

  {
    UnorderedMap const& m = map;

    for (int i = 0; i < n; ++i) {
      map_iterator pos = m.find(i);
      BOOST_TEST(pos != m.end());

      pair const& p = *pos;
      int const v = p.second;

      BOOST_TEST_EQ(v, i);
    }
    BOOST_TEST_EQ(key::count_, n + key_count);

    map_iterator pos = m.find(1337);
    BOOST_TEST(pos == m.end());
    BOOST_TEST_EQ(key::count_, 1 + n + key_count);
  }
}

template <class UnorderedMap> void test_transparent_equal_range()
{
  count_reset();

  UnorderedMap unordered_map;

  // empty tests
  //
  // explicitly test `equal_range()` vs `equal_range() const`
  //
  {
    typedef typename UnorderedMap::iterator iterator;
    typedef std::pair<iterator, iterator> iterator_pair;

    UnorderedMap& map = unordered_map;
    BOOST_TEST(map.empty());

    iterator_pair iters = map.equal_range(0);

    iterator begin = iters.first;
    iterator end = iters.second;

    BOOST_TEST(begin == end);
    BOOST_TEST(begin == map.end());
    BOOST_TEST_EQ(std::distance(begin, end), 0);
  }

  {
    typedef typename UnorderedMap::const_iterator iterator;
    typedef std::pair<iterator, iterator> iterator_pair;

    UnorderedMap const& map = unordered_map;
    BOOST_TEST(map.empty());

    iterator_pair iters = map.equal_range(0);

    iterator begin = iters.first;
    iterator end = iters.second;

    BOOST_TEST(begin == end);
    BOOST_TEST(begin == map.end());
    BOOST_TEST_EQ(std::distance(begin, end), 0);
  }

  BOOST_TEST_EQ(key::count_, 0);

  unordered_map.insert(std::make_pair(0, 1337));
  unordered_map.insert(std::make_pair(1, 1338));
  unordered_map.insert(std::make_pair(2, 1339));
  unordered_map.insert(std::make_pair(0, 1340));
  unordered_map.insert(std::make_pair(0, 1341));
  unordered_map.insert(std::make_pair(0, 1342));

  int const expected_key_count = key::count_;

  // do this so that multimap tests actually test a range with len > 1
  //
  int const expected_range_len = static_cast<int>(unordered_map.size() - 2);

  typedef typename UnorderedMap::value_type value_type;

  // explicitly test `equal_range()` vs `equal_range() const`
  //
  {
    typedef typename UnorderedMap::iterator iterator;
    typedef std::pair<iterator, iterator> iterator_pair;

    UnorderedMap& map = unordered_map;

    iterator_pair iters = map.equal_range(0);

    iterator begin = iters.first;
    iterator end = iters.second;

    BOOST_TEST(begin != end);
    BOOST_TEST(begin != map.end());
    BOOST_TEST_EQ(std::distance(begin, end), expected_range_len);

    for (iterator pos = begin; pos != end; ++pos) {
      value_type const& val = *pos;
      BOOST_TEST_EQ(val.first.x_, 0);
    }

    iters = map.equal_range(1337);

    begin = iters.first;
    end = iters.second;

    BOOST_TEST(begin == end);
    BOOST_TEST(begin == map.end());
    BOOST_TEST_EQ(std::distance(begin, end), 0);

    BOOST_TEST_EQ(key::count_, expected_key_count);
  }

  {
    typedef typename UnorderedMap::const_iterator iterator;
    typedef std::pair<iterator, iterator> iterator_pair;

    UnorderedMap const& map = unordered_map;

    iterator_pair iters = map.equal_range(0);

    iterator begin = iters.first;
    iterator end = iters.second;

    BOOST_TEST(begin != end);
    BOOST_TEST(begin != map.end());
    BOOST_TEST_EQ(std::distance(begin, end), expected_range_len);

    for (iterator pos = begin; pos != end; ++pos) {
      value_type const& val = *begin;
      BOOST_TEST_EQ(val.first.x_, 0);
    }

    iters = map.equal_range(1337);

    begin = iters.first;
    end = iters.second;

    BOOST_TEST(begin == end);
    BOOST_TEST(begin == map.end());
    BOOST_TEST_EQ(std::distance(begin, end), 0);

    BOOST_TEST_EQ(key::count_, expected_key_count);
  }
}

template <class UnorderedMap> void test_non_transparent_equal_range()
{
  count_reset();

  UnorderedMap unordered_map;

  // empty tests
  //
  // explicitly test `equal_range()` vs `equal_range() const`
  //
  {
    typedef typename UnorderedMap::iterator iterator;
    typedef std::pair<iterator, iterator> iterator_pair;

    UnorderedMap& map = unordered_map;
    BOOST_TEST(map.empty());

    iterator_pair iters = map.equal_range(0);

    iterator begin = iters.first;
    iterator end = iters.second;

    BOOST_TEST(begin == end);
    BOOST_TEST(begin == map.end());
    BOOST_TEST_EQ(std::distance(begin, end), 0);
  }

  {
    typedef typename UnorderedMap::const_iterator iterator;
    typedef std::pair<iterator, iterator> iterator_pair;

    UnorderedMap const& map = unordered_map;
    BOOST_TEST(map.empty());

    iterator_pair iters = map.equal_range(0);

    iterator begin = iters.first;
    iterator end = iters.second;

    BOOST_TEST(begin == end);
    BOOST_TEST(begin == map.end());
    BOOST_TEST_EQ(std::distance(begin, end), 0);
  }

  BOOST_TEST_EQ(key::count_, 2);

  unordered_map.insert(std::make_pair(0, 1337));
  unordered_map.insert(std::make_pair(1, 1338));
  unordered_map.insert(std::make_pair(2, 1339));
  unordered_map.insert(std::make_pair(0, 1340));
  unordered_map.insert(std::make_pair(0, 1341));
  unordered_map.insert(std::make_pair(0, 1342));

  int key_count = key::count_;

  // do this so that multimap tests actually test a range with len > 1
  //
  int const expected_range_len = static_cast<int>(unordered_map.size() - 2);

  typedef typename UnorderedMap::value_type value_type;

  // explicitly test `equal_range()` vs `equal_range() const`
  //
  {
    typedef typename UnorderedMap::iterator iterator;
    typedef std::pair<iterator, iterator> iterator_pair;

    UnorderedMap& map = unordered_map;

    iterator_pair iters = map.equal_range(0);

    iterator begin = iters.first;
    iterator end = iters.second;

    BOOST_TEST(begin != end);
    BOOST_TEST(begin != map.end());
    BOOST_TEST_EQ(std::distance(begin, end), expected_range_len);

    for (iterator pos = begin; pos != end; ++pos) {
      value_type const& val = *begin;
      BOOST_TEST_EQ(val.first.x_, 0);
    }

    iters = map.equal_range(1337);

    begin = iters.first;
    end = iters.second;

    BOOST_TEST(begin == end);
    BOOST_TEST(begin == map.end());
    BOOST_TEST_EQ(std::distance(begin, end), 0);

    BOOST_TEST_EQ(key::count_, 2 + key_count);
    key_count += 2;
  }

  {
    typedef typename UnorderedMap::const_iterator iterator;
    typedef std::pair<iterator, iterator> iterator_pair;

    UnorderedMap const& map = unordered_map;

    iterator_pair iters = map.equal_range(0);

    iterator begin = iters.first;
    iterator end = iters.second;

    BOOST_TEST(begin != end);
    BOOST_TEST(begin != map.end());
    BOOST_TEST_EQ(std::distance(begin, end), expected_range_len);

    for (iterator pos = begin; pos != end; ++pos) {
      value_type const& val = *pos;
      BOOST_TEST_EQ(val.first.x_, 0);
    }

    iters = map.equal_range(1337);

    begin = iters.first;
    end = iters.second;

    BOOST_TEST(begin == end);
    BOOST_TEST(begin == map.end());
    BOOST_TEST_EQ(std::distance(begin, end), 0);

    BOOST_TEST_EQ(key::count_, 2 + key_count);
  }
}

template <class UnorderedMap> struct convertible_to_iterator
{
  operator typename UnorderedMap::iterator()
  {
    return typename UnorderedMap::iterator();
  }
};

template <class UnorderedMap> struct convertible_to_const_iterator
{
  operator typename UnorderedMap::const_iterator()
  {
    return typename UnorderedMap::const_iterator();
  }
};

typedef boost::unordered_map<int, int, transparent_hasher,
  transparent_key_equal>
  transparent_unordered_map;

// test that in the presence of the member function template `erase()`, we still
// invoke the correct iterator overloads when the type is implicitly convertible
//
transparent_unordered_map::iterator map_erase_overload_compile_test()
{
  convertible_to_iterator<transparent_unordered_map> c;
  transparent_unordered_map map;
  transparent_unordered_map::iterator pos = map.begin();
  pos = c;
  return map.erase(c);
}

transparent_unordered_map::const_iterator
map_erase_const_overload_compile_test()
{
  convertible_to_const_iterator<transparent_unordered_map> c;
  transparent_unordered_map map;
  transparent_unordered_map::const_iterator pos = map.begin();
  pos = c;
  return map.erase(c);
}

typedef boost::unordered_multimap<int, int, transparent_hasher,
  transparent_key_equal>
  transparent_unordered_multimap;

transparent_unordered_multimap::iterator multimap_erase_overload_compile_test()
{
  convertible_to_iterator<transparent_unordered_multimap> c;
  transparent_unordered_multimap map;
  transparent_unordered_multimap::iterator pos = map.begin();
  pos = c;
  return map.erase(c);
}

transparent_unordered_multimap::const_iterator
multimap_erase_const_overload_compile_test()
{
  convertible_to_const_iterator<transparent_unordered_multimap> c;
  transparent_unordered_multimap map;
  transparent_unordered_multimap::const_iterator pos = map.begin();
  pos = c;
  return map.erase(c);
}

template <class UnorderedMap> void test_transparent_erase()
{
  count_reset();

  UnorderedMap map;

  unsigned long num_erased = 0;

  num_erased = map.erase(0);
  BOOST_TEST(map.empty());
  BOOST_TEST_EQ(num_erased, 0);
  BOOST_TEST_EQ(key::count_, 0);

  map.insert(std::make_pair(0, 1337));
  map.insert(std::make_pair(1, 1338));
  map.insert(std::make_pair(2, 1339));
  map.insert(std::make_pair(0, 1340));
  map.insert(std::make_pair(0, 1341));
  map.insert(std::make_pair(0, 1342));

  BOOST_TEST(map.find(0) != map.end());

  int const expected_key_count = key::count_;
  int const expected_num_erased = static_cast<int>(map.size() - 2);

  num_erased = map.erase(0);
  BOOST_TEST_EQ(num_erased, expected_num_erased);
  BOOST_TEST_EQ(map.size(), 2);
  BOOST_TEST(map.find(0) == map.end());

  num_erased = map.erase(1337);
  BOOST_TEST_EQ(num_erased, 0);
  BOOST_TEST_EQ(map.size(), 2);

  BOOST_TEST_EQ(key::count_, expected_key_count);
}

template <class UnorderedMap> void test_non_transparent_erase()
{
  count_reset();

  UnorderedMap map;

  unsigned long num_erased = 0;

  num_erased = map.erase(0);
  BOOST_TEST(map.empty());
  BOOST_TEST_EQ(num_erased, 0);
  BOOST_TEST_EQ(key::count_, 1);

  map.insert(std::make_pair(0, 1337));
  map.insert(std::make_pair(1, 1338));
  map.insert(std::make_pair(2, 1339));
  map.insert(std::make_pair(0, 1340));
  map.insert(std::make_pair(0, 1341));
  map.insert(std::make_pair(0, 1342));

  int const expected_num_erased = static_cast<int>(map.size() - 2);

  BOOST_TEST(map.find(0) != map.end());

  int key_count = key::count_;

  num_erased = map.erase(0);
  ++key_count;
  BOOST_TEST_EQ(key::count_, key_count);
  BOOST_TEST_EQ(num_erased, expected_num_erased);
  BOOST_TEST_EQ(map.size(), 2);

  BOOST_TEST(map.find(0) == map.end());
  ++key_count;

  BOOST_TEST_EQ(key::count_, key_count);

  num_erased = map.erase(1337);
  ++key_count;
  BOOST_TEST_EQ(num_erased, 0);
  BOOST_TEST_EQ(map.size(), 2);
  BOOST_TEST_EQ(key::count_, key_count);
}

// test that in the presence of the member function template `extract()`, we
// still invoke the correct iterator overloads when the type is implicitly
// convertible
//
transparent_unordered_map::node_type extract_const_overload_compile_test()
{
  convertible_to_const_iterator<transparent_unordered_map> c;
  transparent_unordered_map map;
  transparent_unordered_map::const_iterator pos = map.begin();
  pos = c;
  return map.extract(c);
}

template <class UnorderedMap> void test_transparent_extract()
{
  typedef typename UnorderedMap::node_type node_type;
  typedef typename UnorderedMap::const_iterator const_iterator;

  count_reset();

  UnorderedMap map;

  node_type nh = map.extract(0);
  BOOST_TEST(nh.empty());
  BOOST_TEST_EQ(key::count_, 0);

  map[key(0)] = 1337;
  map[key(1)] = 1338;
  map[key(2)] = 1339;
  BOOST_TEST_EQ(map.size(), 3);

  int const expected_key_count = static_cast<int>(2 * map.size());
  BOOST_TEST_EQ(key::count_, expected_key_count);

  nh = map.extract(1);
  BOOST_TEST_EQ(map.size(), 2);
  BOOST_TEST_EQ(nh.key().x_, 1);
  BOOST_TEST_EQ(nh.mapped(), 1338);

  nh.mapped() = 1340;

  map.insert(boost::move(nh));

  BOOST_TEST(map.size() == 3);

  const_iterator pos = map.find(1);
  BOOST_TEST(pos != map.end());
  BOOST_TEST_EQ(pos->second, 1340);

  nh = map.extract(1337);
  BOOST_TEST(nh.empty());

  BOOST_TEST_EQ(key::count_, expected_key_count);
}

template <class UnorderedMap> void test_non_transparent_extract()
{
  typedef typename UnorderedMap::node_type node_type;
  typedef typename UnorderedMap::const_iterator const_iterator;

  count_reset();

  UnorderedMap map;

  node_type nh = map.extract(0);
  BOOST_TEST(nh.empty());
  BOOST_TEST_EQ(key::count_, 1);

  map[key(0)] = 1337;
  map[key(1)] = 1338;
  map[key(2)] = 1339;
  BOOST_TEST_EQ(map.size(), 3);

  int key_count = 1 + static_cast<int>(2 * map.size());
  BOOST_TEST_EQ(key::count_, key_count);

  nh = map.extract(1);
  ++key_count;
  BOOST_TEST_EQ(map.size(), 2);
  BOOST_TEST_EQ(nh.key().x_, 1);
  BOOST_TEST_EQ(nh.mapped(), 1338);
  BOOST_TEST_EQ(key::count_, key_count);

  nh.mapped() = 1340;

  map.insert(boost::move(nh));

  BOOST_TEST_EQ(map.size(), 3);

  const_iterator pos = map.find(1);
  ++key_count;
  BOOST_TEST(pos != map.end());
  BOOST_TEST_EQ(pos->second, 1340);
  BOOST_TEST_EQ(key::count_, key_count);

  nh = map.extract(1337);
  ++key_count;
  BOOST_TEST(nh.empty());
  BOOST_TEST_EQ(key::count_, key_count);
}

void test_unordered_map()
{
  {
    typedef boost::unordered_map<key, int, transparent_hasher,
      transparent_key_equal>
      unordered_map;

    test_transparent_count<unordered_map>();
    test_transparent_find<unordered_map>();
    test_transparent_equal_range<unordered_map>();
    test_transparent_erase<unordered_map>();
    test_transparent_extract<unordered_map>();
  }

  {
    // non-transparent Hash, non-transparent KeyEqual
    //
    typedef boost::unordered_map<key, int, hasher, key_equal> unordered_map;

    test_non_transparent_count<unordered_map>();
    test_non_transparent_find<unordered_map>();
    test_non_transparent_equal_range<unordered_map>();
    test_non_transparent_erase<unordered_map>();
    test_non_transparent_extract<unordered_map>();
  }

  {
    // transparent Hash, non-transparent KeyEqual
    //
    typedef boost::unordered_map<key, int, transparent_hasher, key_equal>
      unordered_map;

    test_non_transparent_count<unordered_map>();
    test_non_transparent_find<unordered_map>();
    test_non_transparent_equal_range<unordered_map>();
    test_non_transparent_erase<unordered_map>();
    test_non_transparent_extract<unordered_map>();
  }

  {
    // non-transparent Hash, transparent KeyEqual
    //
    typedef boost::unordered_map<key, int, hasher, transparent_key_equal>
      unordered_map;

    test_non_transparent_count<unordered_map>();
    test_non_transparent_find<unordered_map>();
    test_non_transparent_equal_range<unordered_map>();
    test_non_transparent_erase<unordered_map>();
    test_non_transparent_extract<unordered_map>();
  }
}

void test_unordered_multimap()
{
  {
    typedef boost::unordered_multimap<key, int, transparent_hasher,
      transparent_key_equal>
      unordered_multimap;

    test_transparent_find<unordered_multimap>();
    test_transparent_equal_range<unordered_multimap>();
    test_transparent_erase<unordered_multimap>();
  }

  {
    // non-transparent Hash, non-transparent KeyEqual
    //
    typedef boost::unordered_multimap<key, int, hasher, key_equal>
      unordered_multimap;

    test_non_transparent_find<unordered_multimap>();
    test_non_transparent_equal_range<unordered_multimap>();
    test_non_transparent_erase<unordered_multimap>();
  }

  {
    // transparent Hash, non-transparent KeyEqual
    //
    typedef boost::unordered_multimap<key, int, transparent_hasher, key_equal>
      unordered_multimap;

    test_non_transparent_find<unordered_multimap>();
    test_non_transparent_equal_range<unordered_multimap>();
    test_non_transparent_erase<unordered_multimap>();
  }

  {
    // non-transparent Hash, transparent KeyEqual
    //
    typedef boost::unordered_multimap<key, int, hasher, transparent_key_equal>
      unordered_multimap;

    test_non_transparent_find<unordered_multimap>();
    test_non_transparent_equal_range<unordered_multimap>();
    test_non_transparent_erase<unordered_multimap>();
  }
}

UNORDERED_AUTO_TEST (transparent_ops) {
  test_unordered_map();
  test_unordered_multimap();
}

RUN_TESTS()
