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

#include <boost/container_hash/hash.hpp>
#include <boost/core/allocator_access.hpp>
#include <boost/type_traits/type_identity.hpp>

#include <functional>
#include <utility>

namespace boost {
  namespace unordered {
    namespace detail {
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
      using type_policy = detail::concurrent_map_types<Key,T>;

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

      concurrent_flat_map() : concurrent_flat_map(0) {}
      explicit concurrent_flat_map(size_type n, const hasher& hf = hasher(),
        const key_equal& eql = key_equal(),
        const allocator_type& a = allocator_type())
          : table_(n, hf, eql, a)
      {
      }

      bool insert(value_type const& obj)
      {
        return table_.insert(obj);
      }
    };
  } // namespace unordered
} // namespace boost

#endif // BOOST_UNORDERED_CONCURRENT_FLAT_MAP_HPP