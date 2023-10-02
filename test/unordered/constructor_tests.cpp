
// Copyright 2006-2010 Daniel James.
// Copyright (C) 2022-2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../helpers/unordered.hpp"

#include "../helpers/equivalent.hpp"
#include "../helpers/input_iterator.hpp"
#include "../helpers/invariants.hpp"
#include "../helpers/random_values.hpp"
#include "../helpers/test.hpp"
#include "../helpers/tracker.hpp"
#include "../objects/test.hpp"

#include <vector>

namespace constructor_tests {

  test::seed_t initialize_seed(356730);

  template <class T>
  void constructor_tests1(T*, test::random_generator generator)
  {
    typename T::hasher hf;
    typename T::key_equal eq;
    typename T::allocator_type al;

    UNORDERED_SUB_TEST("Construct 1")
    {
      test::check_instances check_;

      T x(0, hf, eq);
      BOOST_TEST(x.empty());
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 2")
    {
      test::check_instances check_;

      T x(100, hf);
      BOOST_TEST(x.empty());
      BOOST_TEST(x.bucket_count() >= 100);
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 3")
    {
      test::check_instances check_;

      T x(2000);
      BOOST_TEST(x.empty());
      BOOST_TEST(x.bucket_count() >= 2000);
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 4")
    {
      test::check_instances check_;

      T x;
      BOOST_TEST(x.empty());
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 5")
    {
      test::check_instances check_;

      test::random_values<T> v(1000, generator);
      T x(v.begin(), v.end(), 10000, hf, eq);
      BOOST_TEST(x.bucket_count() >= 10000);
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_container(x, v);
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 6")
    {
      test::check_instances check_;

      test::random_values<T> v(10, generator);
      T x(v.begin(), v.end(), 10000, hf);
      BOOST_TEST(x.bucket_count() >= 10000);
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_container(x, v);
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 7")
    {
      test::check_instances check_;

      test::random_values<T> v(100, generator);
      T x(v.begin(), v.end(), 100);
      BOOST_TEST(x.bucket_count() >= 100);
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_container(x, v);
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 8")
    {
      test::check_instances check_;

      test::random_values<T> v(1, generator);
      T x(v.begin(), v.end());
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_container(x, v);
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 9")
    {
      test::check_instances check_;

      T x(0, hf, eq, al);
      BOOST_TEST(x.empty());
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 10")
    {
      test::check_instances check_;

      test::random_values<T> v(1000, generator);
      T x(v.begin(), v.end(), 10000, hf, eq, al);
      BOOST_TEST(x.bucket_count() >= 10000);
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_container(x, v);
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 11")
    {
      test::check_instances check_;

      T x(al);
      BOOST_TEST(x.empty());
      BOOST_TEST(test::equivalent(x.hash_function(), hf));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 12")
    {
      test::check_instances check_;

      test::random_values<T> v(1000, generator);
      T x(v.begin(), v.end(), al);
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_container(x, v);
    }
  }

  template <class T>
  void constructor_tests2(T*, test::random_generator const& generator)
  {
    typename T::hasher hf;
    typename T::hasher hf1(1);
    typename T::hasher hf2(2);
    typename T::key_equal eq;
    typename T::key_equal eq1(1);
    typename T::key_equal eq2(2);
    typename T::allocator_type al;
    typename T::allocator_type al1(1);
    typename T::allocator_type al2(2);

    UNORDERED_SUB_TEST("Construct 1")
    {
      test::check_instances check_;
      T x(10000, hf1, eq1);
      BOOST_TEST(x.bucket_count() >= 10000);
      BOOST_TEST(test::equivalent(x.hash_function(), hf1));
      BOOST_TEST(test::equivalent(x.key_eq(), eq1));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 2")
    {
      test::check_instances check_;
      T x(100, hf1);
      BOOST_TEST(x.empty());
      BOOST_TEST(x.bucket_count() >= 100);
      BOOST_TEST(test::equivalent(x.hash_function(), hf1));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 3")
    {
      test::check_instances check_;
      test::random_values<T> v(100, generator);
      T x(v.begin(), v.end(), 0, hf1, eq1);
      BOOST_TEST(test::equivalent(x.hash_function(), hf1));
      BOOST_TEST(test::equivalent(x.key_eq(), eq1));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_container(x, v);
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 4")
    {
      test::check_instances check_;
      test::random_values<T> v(5, generator);
      T x(v.begin(), v.end(), 1000, hf1);
      BOOST_TEST(x.bucket_count() >= 1000);
      BOOST_TEST(test::equivalent(x.hash_function(), hf1));
      BOOST_TEST(test::equivalent(x.key_eq(), eq));
      BOOST_TEST(test::equivalent(x.get_allocator(), al));
      test::check_container(x, v);
      test::check_equivalent_keys(x);
    }

    UNORDERED_SUB_TEST("Construct 5")
    {
      test::check_instances check_;
      test::random_values<T> v(100, generator);
      T x(v.begin(), v.end(), 0, hf, eq, al1);
      T y(x.begin(), x.end(), 0, hf1, eq1, al2);
      test::check_container(x, v);
      test::check_container(y, x);
      test::check_equivalent_keys(x);
      test::check_equivalent_keys(y);
    }

    UNORDERED_SUB_TEST("Construct 6")
    {
      test::check_instances check_;
      test::random_values<T> v(100, generator);
      T x(v.begin(), v.end(), 0, hf1, eq1);
      T y(x.begin(), x.end(), 0, hf, eq);
      test::check_container(x, v);
      test::check_container(y, x);
      test::check_equivalent_keys(x);
      test::check_equivalent_keys(y);
    }

    UNORDERED_SUB_TEST("Construct 7")
    {
      test::check_instances check_;
      test::random_values<T> v(100, generator);
      T x(v.begin(), v.end(), 0, hf1, eq1);
      T y(x.begin(), x.end(), 0, hf2, eq2);
      test::check_container(x, v);
      test::check_container(y, x);
      test::check_equivalent_keys(x);
      test::check_equivalent_keys(y);
    }

    UNORDERED_SUB_TEST("Construct 8 - from input iterator")
    {
      test::check_instances check_;
      test::random_values<T> v(100, generator);
      typename test::random_values<T>::const_iterator v_begin = v.begin(),
                                                      v_end = v.end();
      T x(test::input_iterator(v_begin), test::input_iterator(v_end), 0, hf1,
        eq1);
      typename T::const_iterator x_begin = x.begin(), x_end = x.end();
      T y(test::input_iterator(x_begin), test::input_iterator(x_end), 0, hf2,
        eq2);
      test::check_container(x, v);
      test::check_container(y, x);
      test::check_equivalent_keys(x);
      test::check_equivalent_keys(y);
    }

    UNORDERED_SUB_TEST("Construct 8.5 - from copy iterator")
    {
      test::check_instances check_;
      test::random_values<T> v(100, generator);
      T x(test::copy_iterator(v.begin()), test::copy_iterator(v.end()), 0, hf1,
        eq1);
      T y(test::copy_iterator(x.begin()), test::copy_iterator(x.end()), 0, hf2,
        eq2);
      test::check_container(x, v);
      test::check_container(y, x);
      test::check_equivalent_keys(x);
      test::check_equivalent_keys(y);
    }

    UNORDERED_SUB_TEST("Construct 9")
    {
      test::check_instances check_;

      test::random_values<T> v(100, generator);
      T x(50);
      BOOST_TEST(x.bucket_count() >= 50);
      x.max_load_factor(10);
      BOOST_TEST(x.bucket_count() >= 50);
      x.insert(v.begin(), v.end());
      BOOST_TEST(x.bucket_count() >= 50);
      test::check_container(x, v);
      test::check_equivalent_keys(x);
    }

    typedef typename T::value_type value_type;

    std::initializer_list<value_type> list;

    test::random_values<T> v(3, generator);
    std::vector<value_type> vec(v.begin(), v.end());
    BOOST_ASSERT(vec.size() >= 3);

    // create a new vector here because erase() requires assignability which is
    // deleted for some of the test types
    //
    std::vector<value_type> expected(vec.begin(), vec.begin() + 3);

    UNORDERED_SUB_TEST("Initializer list construct 1")
    {
      test::check_instances check_;

      {
        T x(list);
        BOOST_TEST(x.empty());
        BOOST_TEST(test::equivalent(x.hash_function(), hf));
        BOOST_TEST(test::equivalent(x.key_eq(), eq));
        BOOST_TEST(test::equivalent(x.get_allocator(), al));
      }

      {
        T x{vec[0], vec[1], vec[2]};
        BOOST_TEST_NOT(x.empty());
        BOOST_TEST_GT(x.bucket_count(), 0u);
        BOOST_TEST(test::equivalent(x.hash_function(), hf));
        BOOST_TEST(test::equivalent(x.key_eq(), eq));
        BOOST_TEST(test::equivalent(x.get_allocator(), al));
        test::check_container(x, expected);
      }
    }

    UNORDERED_SUB_TEST("Initializer list construct 2")
    {
      test::check_instances check_;

      {
        T x(list, 1000);
        BOOST_TEST(x.empty());
        BOOST_TEST(x.bucket_count() >= 1000);
        BOOST_TEST(test::equivalent(x.hash_function(), hf));
        BOOST_TEST(test::equivalent(x.key_eq(), eq));
        BOOST_TEST(test::equivalent(x.get_allocator(), al));
      }

      {
        T x({vec[0], vec[1], vec[2]}, 1000);
        BOOST_TEST_NOT(x.empty());
        BOOST_TEST(x.bucket_count() >= 1000);
        BOOST_TEST(test::equivalent(x.hash_function(), hf));
        BOOST_TEST(test::equivalent(x.key_eq(), eq));
        BOOST_TEST(test::equivalent(x.get_allocator(), al));
        test::check_container(x, expected);
      }
    }

    UNORDERED_SUB_TEST("Initializer list construct 3")
    {
      {
        test::check_instances check_;

        T x(list, 10, hf1);
        BOOST_TEST(x.empty());
        BOOST_TEST(x.bucket_count() >= 10);
        BOOST_TEST(test::equivalent(x.hash_function(), hf1));
        BOOST_TEST(test::equivalent(x.key_eq(), eq));
        BOOST_TEST(test::equivalent(x.get_allocator(), al));
      }

      {
        test::check_instances check_;

        T x({vec[0], vec[1], vec[2]}, 10, hf1);
        BOOST_TEST_NOT(x.empty());
        BOOST_TEST(x.bucket_count() >= 10);
        BOOST_TEST(test::equivalent(x.hash_function(), hf1));
        BOOST_TEST(test::equivalent(x.key_eq(), eq));
        BOOST_TEST(test::equivalent(x.get_allocator(), al));
        test::check_container(x, expected);
      }
    }

    UNORDERED_SUB_TEST("Initializer list construct 4")
    {
      {
        test::check_instances check_;

        T x(list, 10, hf1, eq1);
        BOOST_TEST(x.empty());
        BOOST_TEST(x.bucket_count() >= 10);
        BOOST_TEST(test::equivalent(x.hash_function(), hf1));
        BOOST_TEST(test::equivalent(x.key_eq(), eq1));
        BOOST_TEST(test::equivalent(x.get_allocator(), al));
      }

      {
        test::check_instances check_;

        T x({vec[0], vec[1], vec[2]}, 10, hf1, eq1);
        BOOST_TEST_NOT(x.empty());
        BOOST_TEST(x.bucket_count() >= 10);
        BOOST_TEST(test::equivalent(x.hash_function(), hf1));
        BOOST_TEST(test::equivalent(x.key_eq(), eq1));
        BOOST_TEST(test::equivalent(x.get_allocator(), al));
        test::check_container(x, expected);
      }
    }

    UNORDERED_SUB_TEST("Initializer list construct 5")
    {
      {
        test::check_instances check_;

        T x(list, 10, hf1, eq1, al1);
        BOOST_TEST(x.empty());
        BOOST_TEST(x.bucket_count() >= 10);
        BOOST_TEST(test::equivalent(x.hash_function(), hf1));
        BOOST_TEST(test::equivalent(x.key_eq(), eq1));
        BOOST_TEST(test::equivalent(x.get_allocator(), al1));
      }

      {
        test::check_instances check_;

        T x({vec[0], vec[1], vec[2]}, 10, hf1, eq1, al1);
        BOOST_TEST_NOT(x.empty());
        BOOST_TEST(x.bucket_count() >= 10);
        BOOST_TEST(test::equivalent(x.hash_function(), hf1));
        BOOST_TEST(test::equivalent(x.key_eq(), eq1));
        BOOST_TEST(test::equivalent(x.get_allocator(), al1));
        test::check_container(x, expected);
      }
    }

    UNORDERED_SUB_TEST("Initializer list construct 6")
    {
      {
        test::check_instances check_;

        T x(list, 10, al1);
        BOOST_TEST(x.empty());
        BOOST_TEST(x.bucket_count() >= 10);
        BOOST_TEST(test::equivalent(x.get_allocator(), al1));
      }

      {
        test::check_instances check_;

        T x({vec[0], vec[1], vec[2]}, 10, al1);
        BOOST_TEST_NOT(x.empty());
        BOOST_TEST(x.bucket_count() >= 10);
        BOOST_TEST(test::equivalent(x.get_allocator(), al1));
        test::check_container(x, expected);
      }
    }

    UNORDERED_SUB_TEST("Initializer list construct 7")
    {
      {
        test::check_instances check_;

        T x(list, 10, hf1, al1);
        BOOST_TEST(x.empty());
        BOOST_TEST(x.bucket_count() >= 10);
        BOOST_TEST(test::equivalent(x.hash_function(), hf1));
        BOOST_TEST(test::equivalent(x.get_allocator(), al1));
      }

      {
        test::check_instances check_;

        T x({vec[0], vec[1], vec[2]}, 10, hf1, al1);
        BOOST_TEST_NOT(x.empty());
        BOOST_TEST(x.bucket_count() >= 10);
        BOOST_TEST(test::equivalent(x.hash_function(), hf1));
        BOOST_TEST(test::equivalent(x.get_allocator(), al1));
        test::check_container(x, expected);
      }
    }

    UNORDERED_SUB_TEST("Initializer list construct 8")
    {
      test::check_instances check_;

      {
        T x(list, al1);
        BOOST_TEST(x.empty());
        BOOST_TEST(test::equivalent(x.get_allocator(), al1));
      }

      {
        T x({vec[0], vec[1], vec[2]}, al1);
        BOOST_TEST(test::equivalent(x.get_allocator(), al1));
        test::check_container(x, expected);
      }
    }
  }

  template <class T>
  void no_alloc_default_construct_test(T*, test::random_generator)
  {

#ifdef BOOST_UNORDERED_FOA_TESTS
    using allocator_type = typename T::allocator_type;
    using value_type =
      typename boost::allocator_value_type<allocator_type>::type;
    using pointer = typename boost::allocator_pointer<allocator_type>::type;
    static_assert(std::is_same<pointer, value_type*>::value,
      "only raw pointers for this test");
#endif

    UNORDERED_SUB_TEST("Construct 1")
    {
      T x;
      BOOST_TEST_EQ(x.bucket_count(), 0u);
      BOOST_TEST_EQ(test::detail::tracker.count_allocations, 0u);
    }

    UNORDERED_SUB_TEST("Construct 2")
    {
      {
        T x(0);
        BOOST_TEST_EQ(x.bucket_count(), 0u);
        BOOST_TEST_EQ(test::detail::tracker.count_allocations, 0u);
      }

      {
        T x(1);
        BOOST_TEST_GT(x.bucket_count(), 0u);
        BOOST_TEST_GT(test::detail::tracker.count_allocations, 0u);
      }
    }

    UNORDERED_SUB_TEST("Construct 3")
    {
      test::random_values<T> v;
      T x(v.begin(), v.end());
      BOOST_TEST_EQ(x.bucket_count(), 0u);
      BOOST_TEST_EQ(test::detail::tracker.count_allocations, 0u);
    }

    UNORDERED_SUB_TEST("Construct 4")
    {
      {
        test::random_values<T> v;
        T x(v.begin(), v.end(), 0);
        BOOST_TEST_EQ(x.bucket_count(), 0u);
        BOOST_TEST_EQ(test::detail::tracker.count_allocations, 0u);
      }

      {
        test::random_values<T> v;
        T x(v.begin(), v.end(), 1);
        BOOST_TEST_GT(x.bucket_count(), 0u);
        BOOST_TEST_GT(test::detail::tracker.count_allocations, 0u);
      }
    }

    UNORDERED_SUB_TEST("Construct 5")
    {
      typename T::allocator_type al;

      {
        T x(al);
        BOOST_TEST_EQ(x.bucket_count(), 0u);
        BOOST_TEST_EQ(test::detail::tracker.count_allocations, 0u);
      }
    }

    UNORDERED_SUB_TEST("Construct 6")
    {
      typename T::allocator_type al;

      T x(0, al);
      BOOST_TEST_EQ(x.bucket_count(), 0u);
      BOOST_TEST_EQ(test::detail::tracker.count_allocations, 0u);
    }

    UNORDERED_SUB_TEST("Initializer list 1")
    {
      std::initializer_list<typename T::value_type> list;
      T x(list);
      BOOST_TEST_EQ(x.bucket_count(), 0u);
      BOOST_TEST_EQ(test::detail::tracker.count_allocations, 0u);
    }

    UNORDERED_SUB_TEST("Initializer list 2")
    {
      {
        std::initializer_list<typename T::value_type> list;
        T x(list, 0);
        BOOST_TEST_EQ(x.bucket_count(), 0u);
        BOOST_TEST_EQ(test::detail::tracker.count_allocations, 0u);
      }

      {
        std::initializer_list<typename T::value_type> list;
        T x(list, 1);
        BOOST_TEST_GT(x.bucket_count(), 0u);
        BOOST_TEST_GT(test::detail::tracker.count_allocations, 0u);
      }
    }
  }

  template <class T>
  void map_constructor_test(T*, test::random_generator const& generator)
  {
    typedef test::list<
      std::pair<typename T::key_type, typename T::mapped_type> >
      list;
    test::random_values<T> v(1000, generator);
    list l(v.begin(), v.end());
    T x(l.begin(), l.end());

    test::check_container(x, v);
    test::check_equivalent_keys(x);
  }

  static std::size_t counted_pointer_count = 0;

  template <typename T>
  class counted_pointer {
   public:
    counted_pointer(T* p_ = nullptr) : p{p_} {
      ++counted_pointer_count; 
    }
    counted_pointer(counted_pointer const& x) : p{x.p} {
      ++counted_pointer_count; 
    }
    ~counted_pointer() { 
      --counted_pointer_count;
    }

    counted_pointer& operator=(T* p_) {
      p = p_;
      return *this;
    }

    operator T*() const noexcept { return p; }

    template <typename Q = T>
    Q& operator*() const noexcept {
      return *p;
    }

    T* operator->() const noexcept { return p; }
    counted_pointer& operator++() noexcept {
      ++p;
      return *this;
    }
    counted_pointer operator++(int) noexcept {
      auto x = *this;
      ++p;
      ;
      return x;
    }
    counted_pointer& operator+=(std::ptrdiff_t n) noexcept {
      p += n;
      return *this;
    }
    counted_pointer& operator-=(std::ptrdiff_t n) noexcept {
      p -= n;
      return *this;
    }
    friend bool operator==(const counted_pointer& x, const counted_pointer& y)
    {
      return x.p == y.p;
    }
    friend bool operator!=(const counted_pointer& x, const counted_pointer& y)
    {
      return x.p != y.p;
    }
    friend bool operator<(const counted_pointer& x, const counted_pointer& y)
    {
      return x.p < y.p;
    }
    friend bool operator<=(const counted_pointer& x, const counted_pointer& y)
    {
      return x.p <= y.p;
    }
    friend bool operator>(const counted_pointer& x, const counted_pointer& y)
    {
      return x.p > y.p;
    }
    friend bool operator>=(const counted_pointer& x, const counted_pointer& y)
    {
      return x.p >= y.p;
    }

    template <typename Q = T>
    static counted_pointer pointer_to(Q& x) noexcept {
      return std::addressof(x);
    }

   private:
    T* p;
  };

  template <class T>
  struct counted_pointer_allocator {
    using value_type = T;
    using pointer = counted_pointer<T>;

    counted_pointer_allocator() = default;
    template <class U>
    counted_pointer_allocator(const counted_pointer_allocator<U>&) noexcept {}

    template <class U>
    bool operator==(const counted_pointer_allocator<U>&) const noexcept {
      return true;
    }

    template <class U>
    bool operator!=(const counted_pointer_allocator<U>&) const noexcept {
      return false;
    }

    pointer allocate(std::size_t n) const {
      return std::allocator<T>().allocate(n);
    }

    void deallocate(pointer p, std::size_t n) const noexcept {
      std::allocator<T>().deallocate(p, n);
    }
  };

  template <class T>
  void fancy_pointer_noleak_test(T*, test::random_generator const& generator)
  {
    // https://github.com/boostorg/unordered/issues/201

    auto const pointer_count = counted_pointer_count;
    {
      test::random_values<T> v(1000, generator);
      T x(v.begin(), v.end());
      (void)x.begin();
    }
    BOOST_TEST_EQ(pointer_count, counted_pointer_count);
  }

  using test::default_generator;
  using test::generate_collisions;
  using test::limited_range;

#ifdef BOOST_UNORDERED_FOA_TESTS
  boost::unordered_flat_map<test::object, test::object, test::hash,
    test::equal_to, std::allocator<test::object> >* test_map_std_alloc;

  boost::unordered_flat_set<test::object, test::hash, test::equal_to,
    test::allocator1<test::object> >* test_set_raw_ptr;
  boost::unordered_node_set<test::object, test::hash, test::equal_to,
    test::allocator1<test::object> >* test_node_set_raw_ptr;
  boost::unordered_flat_map<test::object, test::object, test::hash,
    test::equal_to, test::allocator1<test::object> >* test_map_raw_ptr;
  boost::unordered_node_map<test::object, test::object, test::hash,
    test::equal_to, test::allocator1<test::object> >* test_node_map_raw_ptr;

  boost::unordered_flat_set<test::object, test::hash, test::equal_to,
    test::allocator1<test::object> >* test_set;
  boost::unordered_node_set<test::object, test::hash, test::equal_to,
    test::allocator1<test::object> >* test_node_set;
  boost::unordered_flat_map<test::object, test::object, test::hash,
    test::equal_to, test::allocator2<test::object> >* test_map;
  boost::unordered_node_map<test::object, test::object, test::hash,
    test::equal_to, test::allocator2<test::object> >* test_node_map;

  boost::unordered_flat_set<test::object, test::hash, test::equal_to,
    counted_pointer_allocator<test::object> >* test_set_counted_pointer;
  boost::unordered_node_set<test::object, test::hash, test::equal_to,
    counted_pointer_allocator<test::object> >* test_node_set_counted_pointer;
  boost::unordered_flat_map<test::object, test::object, test::hash,
    test::equal_to, counted_pointer_allocator<
      std::pair<test::object const,test::object> > >* test_map_counted_pointer;
  boost::unordered_node_map<test::object, test::object, test::hash,
    test::equal_to, counted_pointer_allocator<
      std::pair<test::object const,test::object> >
  >* test_node_map_counted_pointer;

  UNORDERED_TEST(constructor_tests1,
    ((test_map_std_alloc)(test_set)(test_node_set)(test_map)(test_node_map))(
      (default_generator)(generate_collisions)(limited_range)))

  UNORDERED_TEST(constructor_tests2,
    ((test_set)(test_node_set)(test_map)(test_node_map))(
      (default_generator)(generate_collisions)(limited_range)))

  UNORDERED_TEST(map_constructor_test,
    ((test_map_std_alloc)(test_map)(test_node_map))(
      (default_generator)(generate_collisions)(limited_range)))

  UNORDERED_TEST(fancy_pointer_noleak_test,
    ((test_set_counted_pointer)(test_node_set_counted_pointer)
       (test_map_counted_pointer)(test_node_map_counted_pointer))
     ((default_generator)))

  UNORDERED_TEST(no_alloc_default_construct_test,
    ((test_set_raw_ptr)(test_node_set_raw_ptr)(test_map_raw_ptr)(test_node_map_raw_ptr))(
      (default_generator)(generate_collisions)(limited_range)))
#else
  boost::unordered_map<test::object, test::object, test::hash, test::equal_to,
    std::allocator<test::object> >* test_map_std_alloc;

  boost::unordered_set<test::object, test::hash, test::equal_to,
    test::allocator1<test::object> >* test_set;
  boost::unordered_multiset<test::object, test::hash, test::equal_to,
    test::allocator2<test::object> >* test_multiset;
  boost::unordered_map<test::object, test::object, test::hash, test::equal_to,
    test::allocator2<test::object> >* test_map;
  boost::unordered_multimap<test::object, test::object, test::hash,
    test::equal_to, test::allocator1<test::object> >* test_multimap;

  boost::unordered_set<test::object, test::hash, test::equal_to,
    counted_pointer_allocator<test::object> >* test_set_counted_pointer;
  boost::unordered_multiset<test::object, test::hash, test::equal_to,
    counted_pointer_allocator<test::object> >* test_multiset_counted_pointer;
  boost::unordered_map<test::object, test::object, test::hash,
    test::equal_to, counted_pointer_allocator<
      std::pair<test::object const,test::object> > >* test_map_counted_pointer;
  boost::unordered_multimap<test::object, test::object, test::hash,
    test::equal_to, counted_pointer_allocator<
      std::pair<test::object const,test::object> >
  >* test_multimap_counted_pointer;

  UNORDERED_TEST(constructor_tests1,
    ((test_map_std_alloc)(test_set)(test_multiset)(test_map)(test_multimap))(
      (default_generator)(generate_collisions)(limited_range)))

  UNORDERED_TEST(constructor_tests2,
    ((test_set)(test_multiset)(test_map)(test_multimap))(
      (default_generator)(generate_collisions)(limited_range)))

  UNORDERED_TEST(map_constructor_test,
    ((test_map_std_alloc)(test_map)(test_multimap))(
      (default_generator)(generate_collisions)(limited_range)))

  UNORDERED_TEST(fancy_pointer_noleak_test,
    ((test_set_counted_pointer)(test_multiset_counted_pointer)
       (test_map_counted_pointer)(test_multimap_counted_pointer))
     ((default_generator)))

  UNORDERED_TEST(no_alloc_default_construct_test,
    ((test_set)(test_multiset)(test_map)(test_multimap))(
      (default_generator)(generate_collisions)(limited_range)))
#endif

  UNORDERED_AUTO_TEST (test_default_initializer_list) {
    std::initializer_list<int> init;
#ifdef BOOST_UNORDERED_FOA_TESTS
    boost::unordered_flat_set<int> x1 = init;
    boost::unordered_node_set<int> x2 = init;
    BOOST_TEST(x2.empty());
#else
    boost::unordered_set<int> x1 = init;
#endif
    BOOST_TEST(x1.empty());
  }

  UNORDERED_AUTO_TEST (test_initializer_list) {
#ifdef BOOST_UNORDERED_FOA_TESTS
    boost::unordered_flat_set<int> x1 = {2, 10, 45, -5};
    boost::unordered_node_set<int> x2 = {2, 10, 45, -5};
    BOOST_TEST(x2.find(10) != x2.end());
    BOOST_TEST(x2.find(46) == x2.end());
#else
    boost::unordered_set<int> x1 = {2, 10, 45, -5};
#endif

    BOOST_TEST(x1.find(10) != x1.end());
    BOOST_TEST(x1.find(46) == x1.end());
  }
} // namespace constructor_tests

RUN_TESTS_QUIET()
