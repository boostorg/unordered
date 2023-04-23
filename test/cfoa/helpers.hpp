#ifndef BOOST_UNORDERED_TEST_CFOA_HELPERS_HPP
#define BOOST_UNORDERED_TEST_CFOA_HELPERS_HPP

#include "../helpers/generators.hpp"
#include "../helpers/test.hpp"

#include <boost/container_hash/hash.hpp>
#include <boost/core/span.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

constexpr std::size_t const num_threads = 16;

struct transp_hash
{
  using is_transparent = void;

  template <class T> std::size_t operator()(T const& t) const noexcept
  {
    // std::this_thread::yield();
    return boost::hash<T>()(t);
  }
};

struct transp_key_equal
{
  using is_transparent = void;

  template <class T, class U> bool operator()(T const& lhs, U const& rhs) const
  {
    // std::this_thread::yield();
    return lhs == rhs;
  }
};

struct stateful_hash
{
  int x_ = -1;

  stateful_hash() = default;
  stateful_hash(stateful_hash const&) = default;
  stateful_hash(stateful_hash&& rhs) noexcept
  {
    auto tmp = x_;
    x_ = rhs.x_;
    rhs.x_ = tmp;
  }

  stateful_hash(int const x) : x_{x} {}

  template <class T> std::size_t operator()(T const& t) const noexcept
  {
    std::size_t h = static_cast<std::size_t>(x_);
    boost::hash_combine(h, t);
    // std::this_thread::yield();
    return h;
  }

  bool operator==(stateful_hash const& rhs) const { return x_ == rhs.x_; }

  friend std::ostream& operator<<(std::ostream& os, stateful_hash const& rhs)
  {
    os << "{ x_: " << rhs.x_ << " }";
    return os;
  }

  friend void swap(stateful_hash& lhs, stateful_hash& rhs) noexcept
  {
    if (&lhs != &rhs) {
      std::swap(lhs.x_, rhs.x_);
    }
  }
};

struct stateful_key_equal
{
  int x_ = -1;

  stateful_key_equal() = default;
  stateful_key_equal(stateful_key_equal const&) = default;
  stateful_key_equal(stateful_key_equal&& rhs) noexcept
  {
    auto tmp = x_;
    x_ = rhs.x_;
    rhs.x_ = tmp;
  }

  stateful_key_equal(int const x) : x_{x} {}

  template <class T, class U> bool operator()(T const& t, U const& u) const
  {
    // std::this_thread::yield();
    return t == u;
  }

  bool operator==(stateful_key_equal const& rhs) const { return x_ == rhs.x_; }

  friend std::ostream& operator<<(
    std::ostream& os, stateful_key_equal const& rhs)
  {
    os << "{ x_: " << rhs.x_ << " }";
    return os;
  }

  friend void swap(stateful_key_equal& lhs, stateful_key_equal& rhs) noexcept
  {
    if (&lhs != &rhs) {
      std::swap(lhs.x_, rhs.x_);
    }
  }
};

template <class T> struct stateful_allocator
{
  int x_ = -1;

  using value_type = T;

  stateful_allocator() = default;
  stateful_allocator(stateful_allocator const&) = default;
  stateful_allocator(stateful_allocator&&) = default;

  stateful_allocator(int const x) : x_{x} {}

  template <class U>
  stateful_allocator(stateful_allocator<U> const& rhs) : x_{rhs.x_}
  {
  }

  T* allocate(std::size_t n)
  {
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, std::size_t) { ::operator delete(p); }

  bool operator==(stateful_allocator const& rhs) const { return x_ == rhs.x_; }
  bool operator!=(stateful_allocator const& rhs) const { return x_ != rhs.x_; }
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
  std::mutex m;
  std::condition_variable cv;
  std::size_t c = 0;

  std::vector<std::thread> threads;
  auto subslices = split<T>(values, num_threads);

  for (std::size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back([&f, &subslices, i, &m, &cv, &c] {
      {
        std::unique_lock<std::mutex> lk(m);
        ++c;
        if (c == num_threads) {
          lk.unlock();
          cv.notify_all();
        } else {
          cv.wait(lk, [&] { return c == num_threads; });
        }
      }

      auto s = subslices[i];
      f(s);
    });
  }

  for (auto& t : threads) {
    t.join();
  }
}

template <class X, class Y>
void test_matches_reference(X const& x, Y const& reference_map)
{
  using value_type = typename X::value_type;
  BOOST_TEST_EQ(x.size(), x.visit_all([&](value_type const& kv) {
    BOOST_TEST(reference_map.contains(kv.first));
    BOOST_TEST_EQ(kv.second, reference_map.find(kv.first)->second);
  }));
}

template <class X, class Y>
void test_fuzzy_matches_reference(
  X const& x, Y const& reference_map, test::random_generator rg)
{
  using value_type = typename X::value_type;
  BOOST_TEST_EQ(x.size(), x.visit_all([&](value_type const& kv) {
    BOOST_TEST(reference_map.contains(kv.first));
    if (rg == test::sequential) {
      BOOST_TEST_EQ(kv.second, reference_map.find(kv.first)->second);
    }
  }));
}

template <class T> using span_value_type = typename T::value_type;

void check_raii_counts()
{
  BOOST_TEST_GE(raii::default_constructor, 0u);
  BOOST_TEST_GE(raii::copy_constructor, 0u);
  BOOST_TEST_GE(raii::move_constructor, 0u);
  BOOST_TEST_GT(raii::destructor, 0u);

  BOOST_TEST_EQ(
    raii::default_constructor + raii::copy_constructor + raii::move_constructor,
    raii::destructor);
}

#endif // BOOST_UNORDERED_TEST_CFOA_HELPERS_HPP
