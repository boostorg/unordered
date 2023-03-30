#ifndef BOOST_UNORDERED_TEST_CFOA_HELPERS_HPP
#define BOOST_UNORDERED_TEST_CFOA_HELPERS_HPP

#include "../helpers/generators.hpp"
#include "../helpers/test.hpp"

#include <boost/container_hash/hash.hpp>
#include <boost/core/span.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

#include <atomic>
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>

constexpr std::size_t const num_threads = 16;

struct transp_hash
{
  using is_transparent = void;

  template <class T> std::size_t operator()(T const& t) const noexcept
  {
    return boost::hash<T>()(t);
  }
};

struct transp_key_equal
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

#endif // BOOST_UNORDERED_TEST_CFOA_HELPERS_HPP