#ifndef BOOST_UNORDERED_UNORDERED_FLAT_MAP_HPP_INCLUDED
#define BOOST_UNORDERED_UNORDERED_FLAT_MAP_HPP_INCLUDED

#include <boost/config.hpp>
#if defined(BOOST_HAS_PRAGMA_ONCE)
#pragma once
#endif

#include <boost/unordered/detail/foa.hpp>

#include <boost/core/allocator_access.hpp>

#include <utility>

namespace boost {
  namespace unordered {
    template <class Key, class T, class Hash, class KeyEqual, class Allocator>
    class unordered_flat_map
    {
      struct map_types
      {
        using key_type = Key;
        using value_type = std::pair<Key const, T>;
        static Key const& extract(value_type const& kv) { return kv.first; }
      };

      using table_type = detail::foa::table<map_types, Hash, KeyEqual,
        typename boost::allocator_rebind<Allocator,
          typename map_types::value_type>::type>;

      table_type table_;

    public:
      using key_type = Key;
      using mapped_type = T;
      using value_type = typename map_types::value_type;
      using size_type = std::size_t;
      using key_equal = KeyEqual;
      using iterator = typename table_type::iterator;
      using const_iterator = typename table_type::const_iterator;

      iterator begin() noexcept { return table_.begin(); }
      const_iterator begin() const noexcept { return table_.begin(); }
      const_iterator cbegin() const noexcept { return table_.cbegin(); }

      iterator end() noexcept { return table_.end(); }
      const_iterator end() const noexcept { return table_.end(); }
      const_iterator cend() const noexcept { return table_.cend(); }

      size_type size() const noexcept { return table_.size(); }

      /// Modifiers
      ///

      std::pair<iterator, bool> insert(value_type const& value)
      {
        return table_.insert(value);
      }

      std::pair<iterator, bool> insert(value_type&& value)
      {
        return table_.insert(std::move(value));
      }

      size_type count(key_type const& key) const
      {
        auto pos = table_.find(key);
        return pos != table_.end() ? 1 : 0;
      }

      std::pair<iterator, iterator> equal_range(key_type const& key)
      {
        auto pos = table_.find(key);
        if (pos == table_.end()) {
          return {pos, pos};
        }

        auto next = pos;
        ++next;
        return {pos, next};
      }

      std::pair<const_iterator, const_iterator> equal_range(
        key_type const& key) const
      {
        auto pos = table_.find(key);
        if (pos == table_.end()) {
          return {pos, pos};
        }

        auto next = pos;
        ++next;
        return {pos, next};
      }

      size_type bucket_count() const noexcept { return table_.capacity(); }

      float load_factor() const noexcept { return table_.load_factor(); }

      float max_load_factor() const noexcept
      {
        return table_.max_load_factor();
      }

      key_equal key_eq() const { return table_.key_eq(); }
    };
  } // namespace unordered
} // namespace boost

#endif
