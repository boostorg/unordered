/* Fast open-addressing concurrent hash table.
 *
 * Copyright 2023 Christian Mazakas.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

/* Reference:
 * https://github.com/joaquintides/concurrent_hashmap_api#proposed-synopsis
 */

#ifndef BOOST_UNORDERED_CONCURRENT_FLAT_MAP_HPP
#define BOOST_UNORDERED_CONCURRENT_FLAT_MAP_HPP

#include <boost/unordered/detail/foa/concurrent_table.hpp>
#include <boost/unordered/detail/type_traits.hpp>

#include <boost/container_hash/hash.hpp>
#include <boost/core/allocator_access.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>
#include <boost/type_traits/type_identity.hpp>

#include <functional>
#include <type_traits>
#include <utility>

#define BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)                             \
  static_assert(boost::unordered::detail::is_invocable<F, value_type&>::value, \
    "The provided Callable must be invocable with `value_type&`");

#define BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)                       \
  static_assert(                                                               \
    boost::unordered::detail::is_invocable<F, value_type const&>::value,       \
    "The provided Callable must be invocable with `value_type const&`");

#define BOOST_UNORDERED_COMMA ,

#define BOOST_UNORDERED_LAST_ARG(Arg, Args)                                    \
  mp11::mp_back<mp11::mp_list<Arg BOOST_UNORDERED_COMMA Args> >

#define BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_INVOCABLE(Arg, Args)            \
  BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(BOOST_UNORDERED_LAST_ARG(Arg, Args))

#define BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_CONST_INVOCABLE(Arg, Args)      \
  BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(                               \
    BOOST_UNORDERED_LAST_ARG(Arg, Args))

namespace boost {
  namespace unordered {
    namespace detail {

      template <class F, class... Args>
      struct is_invocable
          : std::is_constructible<std::function<void(Args...)>,
              std::reference_wrapper<typename std::remove_reference<F>::type> >
      {
      };

      template <class Key, class T> struct concurrent_map_types
      {
        using key_type = Key;
        using raw_key_type = typename std::remove_const<Key>::type;
        using raw_mapped_type = typename std::remove_const<T>::type;

        using init_type = std::pair<raw_key_type, raw_mapped_type>;
        using moved_type = std::pair<raw_key_type&&, raw_mapped_type&&>;
        using value_type = std::pair<Key const, T>;

        using element_type = value_type;

        static value_type& value_from(element_type& x) { return x; }

        template <class K, class V>
        static raw_key_type const& extract(std::pair<K, V> const& kv)
        {
          return kv.first;
        }

        static moved_type move(init_type& x)
        {
          return {std::move(x.first), std::move(x.second)};
        }

        static moved_type move(element_type& x)
        {
          // TODO: we probably need to launder here
          return {std::move(const_cast<raw_key_type&>(x.first)),
            std::move(const_cast<raw_mapped_type&>(x.second))};
        }

        template <class A, class... Args>
        static void construct(A& al, init_type* p, Args&&... args)
        {
          boost::allocator_construct(al, p, std::forward<Args>(args)...);
        }

        template <class A, class... Args>
        static void construct(A& al, value_type* p, Args&&... args)
        {
          boost::allocator_construct(al, p, std::forward<Args>(args)...);
        }

        template <class A> static void destroy(A& al, init_type* p) noexcept
        {
          boost::allocator_destroy(al, p);
        }

        template <class A> static void destroy(A& al, value_type* p) noexcept
        {
          boost::allocator_destroy(al, p);
        }
      };
    } // namespace detail

    template <class Key, class T, class Hash = boost::hash<Key>,
      class Pred = std::equal_to<Key>,
      class Allocator = std::allocator<std::pair<Key const, T> > >
    class concurrent_flat_map
    {
    private:
      using type_policy = detail::concurrent_map_types<Key, T>;

      detail::foa::concurrent_table<type_policy, Hash, Pred, Allocator> table_;

    public:
      using key_type = Key;
      using mapped_type = T;
      using value_type = typename type_policy::value_type;
      using init_type = typename type_policy::init_type;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using hasher = typename boost::type_identity<Hash>::type;
      using key_equal = typename boost::type_identity<Pred>::type;
      using allocator_type = typename boost::type_identity<Allocator>::type;
      using reference = value_type&;
      using const_reference = value_type const&;
      using pointer = typename boost::allocator_pointer<allocator_type>::type;
      using const_pointer =
        typename boost::allocator_const_pointer<allocator_type>::type;

      concurrent_flat_map()
          : concurrent_flat_map(detail::foa::default_bucket_count)
      {
      }

      explicit concurrent_flat_map(size_type n, const hasher& hf = hasher(),
        const key_equal& eql = key_equal(),
        const allocator_type& a = allocator_type())
          : table_(n, hf, eql, a)
      {
      }

      template <class InputIterator>
      concurrent_flat_map(InputIterator f, InputIterator l,
        size_type n = detail::foa::default_bucket_count,
        const hasher& hf = hasher(), const key_equal& eql = key_equal(),
        const allocator_type& a = allocator_type())
          : table_(n, hf, eql, a)
      {
        this->insert(f, l);
      }

      concurrent_flat_map(concurrent_flat_map const& rhs)
          : table_(rhs.table_,
              boost::allocator_select_on_container_copy_construction(
                rhs.get_allocator()))
      {
      }

      concurrent_flat_map(concurrent_flat_map&& rhs)
          : table_(std::move(rhs.table_))
      {
      }

      template <class InputIterator>
      concurrent_flat_map(InputIterator f, InputIterator l, allocator_type a)
          : concurrent_flat_map(f, l, 0, hasher(), key_equal(), a)
      {
      }

      explicit concurrent_flat_map(allocator_type a)
          : table_(detail::foa::default_bucket_count, hasher(), key_equal(), a)
      {
      }

      concurrent_flat_map(concurrent_flat_map const& rhs, allocator_type a)
          : table_(rhs.table_, a)
      {
      }

      concurrent_flat_map(concurrent_flat_map&& rhs, allocator_type a)
          : table_(std::move(rhs.table_), a)
      {
      }

      concurrent_flat_map(std::initializer_list<value_type> il,
        size_type n = detail::foa::default_bucket_count,
        const hasher& hf = hasher(), const key_equal& eql = key_equal(),
        const allocator_type& a = allocator_type())
          : concurrent_flat_map(n, hf, eql, a)
      {
        this->insert(il.begin(), il.end());
      }

      concurrent_flat_map(size_type n, const allocator_type& a)
          : concurrent_flat_map(n, hasher(), key_equal(), a)
      {
      }

      concurrent_flat_map(
        size_type n, const hasher& hf, const allocator_type& a)
          : concurrent_flat_map(n, hf, key_equal(), a)
      {
      }

      template <typename InputIterator>
      concurrent_flat_map(
        InputIterator f, InputIterator l, size_type n, const allocator_type& a)
          : concurrent_flat_map(f, l, n, hasher(), key_equal(), a)
      {
      }

      template <typename InputIterator>
      concurrent_flat_map(InputIterator f, InputIterator l, size_type n,
        const hasher& hf, const allocator_type& a)
          : concurrent_flat_map(f, l, n, hf, key_equal(), a)
      {
      }

      /// Capacity
      ///

      size_type size() const noexcept { return table_.size(); }

      BOOST_ATTRIBUTE_NODISCARD bool empty() const noexcept
      {
        return size() == 0;
      }

      template <class F>
      BOOST_FORCEINLINE std::size_t visit(key_type const& k, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)
        return table_.visit(k, f);
      }

      template <class F>
      BOOST_FORCEINLINE std::size_t visit(key_type const& k, F f) const
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        return table_.visit(k, f);
      }

      template <class F>
      BOOST_FORCEINLINE std::size_t cvisit(key_type const& k, F f) const
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        return table_.visit(k, f);
      }

      template <class K, class F>
      BOOST_FORCEINLINE typename std::enable_if<
        detail::are_transparent<K, hasher, key_equal>::value, std::size_t>::type
      visit(K&& k, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)
        return table_.visit(std::forward<K>(k), f);
      }

      template <class K, class F>
      BOOST_FORCEINLINE typename std::enable_if<
        detail::are_transparent<K, hasher, key_equal>::value, std::size_t>::type
      visit(K&& k, F f) const
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        return table_.visit(std::forward<K>(k), f);
      }

      template <class K, class F>
      BOOST_FORCEINLINE typename std::enable_if<
        detail::are_transparent<K, hasher, key_equal>::value, std::size_t>::type
      cvisit(K&& k, F f) const
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        return table_.visit(std::forward<K>(k), f);
      }

      template <class F> BOOST_FORCEINLINE std::size_t visit_all(F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)
        return table_.visit_all(f);
      }

      template <class F> BOOST_FORCEINLINE std::size_t visit_all(F f) const
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        return table_.visit_all(f);
      }

      template <class F> BOOST_FORCEINLINE std::size_t cvisit_all(F f) const
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        return table_.cvisit_all(f);
      }

#if defined(BOOST_UNORDERED_PARALLEL_ALGORITHMS)
      template <class ExecPolicy, class F>
      BOOST_FORCEINLINE
        typename std::enable_if<detail::is_execution_policy<ExecPolicy>::value,
          void>::type
        visit_all(ExecPolicy p, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)
        table_.visit_all(p, f);
      }

      template <class ExecPolicy, class F>
      BOOST_FORCEINLINE
        typename std::enable_if<detail::is_execution_policy<ExecPolicy>::value,
          void>::type
        visit_all(ExecPolicy p, F f) const
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        table_.visit_all(p, f);
      }

      template <class ExecPolicy, class F>
      BOOST_FORCEINLINE
        typename std::enable_if<detail::is_execution_policy<ExecPolicy>::value,
          void>::type
        cvisit_all(ExecPolicy p, F f) const
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        table_.cvisit_all(p, f);
      }
#endif

      /// Modifiers
      ///

      BOOST_FORCEINLINE bool insert(value_type const& obj)
      {
        return table_.insert(obj);
      }
      BOOST_FORCEINLINE bool insert(value_type&& obj)
      {
        return table_.insert(std::move(obj));
      }

      BOOST_FORCEINLINE bool insert(init_type const& obj)
      {
        return table_.insert(obj);
      }
      BOOST_FORCEINLINE bool insert(init_type&& obj)
      {
        return table_.insert(std::move(obj));
      }

      template <class InputIterator>
      BOOST_FORCEINLINE void insert(InputIterator begin, InputIterator end)
      {
        for (auto pos = begin; pos != end; ++pos) {
          table_.insert(*pos);
        }
      }

      BOOST_FORCEINLINE void insert(std::initializer_list<value_type> ilist)
      {
        this->insert(ilist.begin(), ilist.end());
      }

      template <class M>
      BOOST_FORCEINLINE bool insert_or_assign(key_type const& k, M&& obj)
      {
        return table_.try_emplace_or_visit(k, std::forward<M>(obj),
          [&](value_type& m) { m.second = std::forward<M>(obj); });
      }

      template <class M>
      BOOST_FORCEINLINE bool insert_or_assign(key_type&& k, M&& obj)
      {
        return table_.try_emplace_or_visit(std::move(k), std::forward<M>(obj),
          [&](value_type& m) { m.second = std::forward<M>(obj); });
      }

      template <class K, class M>
      BOOST_FORCEINLINE typename std::enable_if<
        detail::are_transparent<K, hasher, key_equal>::value, bool>::type
      insert_or_assign(K&& k, M&& obj)
      {
        return table_.try_emplace_or_visit(std::forward<K>(k),
          std::forward<M>(obj),
          [&](value_type& m) { m.second = std::forward<M>(obj); });
      }

      template <class F>
      BOOST_FORCEINLINE bool insert_or_visit(value_type const& obj, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)
        return table_.insert_or_visit(obj, f);
      }

      template <class F>
      BOOST_FORCEINLINE bool insert_or_visit(value_type&& obj, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)
        return table_.insert_or_visit(std::move(obj), f);
      }

      template <class F>
      BOOST_FORCEINLINE bool insert_or_visit(init_type const& obj, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)
        return table_.insert_or_visit(obj, f);
      }

      template <class F>
      BOOST_FORCEINLINE bool insert_or_visit(init_type&& obj, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)
        return table_.insert_or_visit(std::move(obj), f);
      }

      template <class InputIterator, class F>
      BOOST_FORCEINLINE void insert_or_visit(
        InputIterator first, InputIterator last, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)
        for (; first != last; ++first) {
          table_.insert_or_visit(*first, f);
        }
      }

      template <class F>
      BOOST_FORCEINLINE void insert_or_visit(
        std::initializer_list<value_type> ilist, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)
        this->insert_or_visit(ilist.begin(), ilist.end(), f);
      }

      template <class F>
      BOOST_FORCEINLINE bool insert_or_cvisit(value_type const& obj, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        return table_.insert_or_cvisit(obj, f);
      }

      template <class F>
      BOOST_FORCEINLINE bool insert_or_cvisit(value_type&& obj, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        return table_.insert_or_cvisit(std::move(obj), f);
      }

      template <class F>
      BOOST_FORCEINLINE bool insert_or_cvisit(init_type const& obj, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        return table_.insert_or_cvisit(obj, f);
      }

      template <class F>
      BOOST_FORCEINLINE bool insert_or_cvisit(init_type&& obj, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        return table_.insert_or_cvisit(std::move(obj), f);
      }

      template <class InputIterator, class F>
      BOOST_FORCEINLINE void insert_or_cvisit(
        InputIterator first, InputIterator last, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        for (; first != last; ++first) {
          table_.insert_or_cvisit(*first, f);
        }
      }

      template <class F>
      BOOST_FORCEINLINE void insert_or_cvisit(
        std::initializer_list<value_type> ilist, F f)
      {
        BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)
        this->insert_or_visit(ilist.begin(), ilist.end(), f);
      }

      template <class... Args> BOOST_FORCEINLINE bool emplace(Args&&... args)
      {
        return table_.emplace(std::forward<Args>(args)...);
      }

      template <class Arg, class... Args>
      BOOST_FORCEINLINE bool emplace_or_visit(Arg&& arg, Args&&... args)
      {
        BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_INVOCABLE(Arg, Args...)
        return table_.emplace_or_visit(
          std::forward<Arg>(arg), std::forward<Args>(args)...);
      }

      template <class Arg, class... Args>
      BOOST_FORCEINLINE bool emplace_or_cvisit(Arg&& arg, Args&&... args)
      {
        BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_CONST_INVOCABLE(Arg, Args...)
        return table_.emplace_or_cvisit(
          std::forward<Arg>(arg), std::forward<Args>(args)...);
      }

      template <class... Args>
      BOOST_FORCEINLINE bool try_emplace(key_type const& k, Args&&... args)
      {
        return table_.try_emplace(k, std::forward<Args>(args)...);
      }

      template <class... Args>
      BOOST_FORCEINLINE bool try_emplace(key_type&& k, Args&&... args)
      {
        return table_.try_emplace(std::move(k), std::forward<Args>(args)...);
      }

      template <class K, class... Args>
      BOOST_FORCEINLINE typename std::enable_if<
        detail::are_transparent<K, hasher, key_equal>::value, bool>::type
      try_emplace(K&& k, Args&&... args)
      {
        return table_.try_emplace(
          std::forward<K>(k), std::forward<Args>(args)...);
      }

      template <class Arg, class... Args>
      BOOST_FORCEINLINE bool try_emplace_or_visit(
        key_type const& k, Arg&& arg, Args&&... args)
      {
        BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_INVOCABLE(Arg, Args...)
        return table_.try_emplace_or_visit(
          k, std::forward<Arg>(arg), std::forward<Args>(args)...);
      }

      template <class Arg, class... Args>
      BOOST_FORCEINLINE bool try_emplace_or_cvisit(
        key_type const& k, Arg&& arg, Args&&... args)
      {
        BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_CONST_INVOCABLE(Arg, Args...)
        return table_.try_emplace_or_cvisit(
          k, std::forward<Arg>(arg), std::forward<Args>(args)...);
      }

      template <class Arg, class... Args>
      BOOST_FORCEINLINE bool try_emplace_or_visit(
        key_type&& k, Arg&& arg, Args&&... args)
      {
        BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_INVOCABLE(Arg, Args...)
        return table_.try_emplace_or_visit(
          std::move(k), std::forward<Arg>(arg), std::forward<Args>(args)...);
      }

      template <class Arg, class... Args>
      BOOST_FORCEINLINE bool try_emplace_or_cvisit(
        key_type&& k, Arg&& arg, Args&&... args)
      {
        BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_CONST_INVOCABLE(Arg, Args...)
        return table_.try_emplace_or_cvisit(
          std::move(k), std::forward<Arg>(arg), std::forward<Args>(args)...);
      }

      template <class K, class Arg, class... Args>
      BOOST_FORCEINLINE bool try_emplace_or_visit(
        K&& k, Arg&& arg, Args&&... args)
      {
        BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_INVOCABLE(Arg, Args...)
        return table_.try_emplace_or_visit(std::forward<K>(k),
          std::forward<Arg>(arg), std::forward<Args>(args)...);
      }

      template <class K, class Arg, class... Args>
      BOOST_FORCEINLINE bool try_emplace_or_cvisit(
        K&& k, Arg&& arg, Args&&... args)
      {
        BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_CONST_INVOCABLE(Arg, Args...)
        return table_.try_emplace_or_cvisit(std::forward<K>(k),
          std::forward<Arg>(arg), std::forward<Args>(args)...);
      }

      BOOST_FORCEINLINE size_type erase(key_type const& k)
      {
        return table_.erase(k);
      }

      template <class K>
      BOOST_FORCEINLINE typename std::enable_if<
        detail::are_transparent<K, hasher, key_equal>::value, size_type>::type
      erase(K&& k)
      {
        return table_.erase(std::forward<K>(k));
      }

      template <class F>
      BOOST_FORCEINLINE size_type erase_if(key_type const& k, F f)
      {
        return table_.erase_if(k, f);
      }

      template <class K, class F>
      BOOST_FORCEINLINE typename std::enable_if<
        detail::are_transparent<K, hasher, key_equal>::value &&
          !detail::is_execution_policy<K>::value,
        size_type>::type
      erase_if(K&& k, F f)
      {
        return table_.erase_if(std::forward<K>(k), f);
      }

#if defined(BOOST_UNORDERED_PARALLEL_ALGORITHMS)
      template <class ExecPolicy, class F>
      BOOST_FORCEINLINE
        typename std::enable_if<detail::is_execution_policy<ExecPolicy>::value,
          void>::type
        erase_if(ExecPolicy p, F f)
      {
        table_.erase_if(p, f);
      }
#endif

      template <class F> BOOST_FORCEINLINE size_type erase_if(F f)
      {
        return table_.erase_if(f);
      }

      /// Hash Policy
      ///
      void rehash(size_type n) { table_.rehash(n); }
      void reserve(size_type n) { table_.reserve(n); }

      /// Observers
      ///
      allocator_type get_allocator() const noexcept
      {
        return table_.get_allocator();
      }

      hasher hash_function() const { return table_.hash_function(); }
      key_equal key_eq() const { return table_.key_eq(); }
    };
  } // namespace unordered
} // namespace boost

#undef BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE
#undef BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE
#undef BOOST_UNORDERED_COMMA
#undef BOOST_UNORDERED_LAST_ARG
#undef BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_INVOCABLE
#undef BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_CONST_INVOCABLE

#endif // BOOST_UNORDERED_CONCURRENT_FLAT_MAP_HPP
