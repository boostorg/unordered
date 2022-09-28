// Copyright (C) 2022 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNORDERED_UNORDERED_FLAT_MAP_HPP_INCLUDED
#define BOOST_UNORDERED_UNORDERED_FLAT_MAP_HPP_INCLUDED

#include <boost/config.hpp>
#if defined(BOOST_HAS_PRAGMA_ONCE)
#pragma once
#endif

#include <boost/unordered/detail/foa.hpp>
#include <boost/unordered/unordered_flat_map_fwd.hpp>

#include <boost/core/allocator_access.hpp>

#include <initializer_list>
#include <iterator>
#include <type_traits>
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
      using reference = value_type&;
      using const_reference = value_type const&;
      using iterator = typename table_type::iterator;
      using const_iterator = typename table_type::const_iterator;

      iterator begin() noexcept { return table_.begin(); }
      const_iterator begin() const noexcept { return table_.begin(); }
      const_iterator cbegin() const noexcept { return table_.cbegin(); }

      iterator end() noexcept { return table_.end(); }
      const_iterator end() const noexcept { return table_.end(); }
      const_iterator cend() const noexcept { return table_.cend(); }

      /// Capacity
      ///

      BOOST_ATTRIBUTE_NODISCARD bool empty() const noexcept
      {
        return table_.empty();
      }

      size_type size() const noexcept { return table_.size(); }

      /// Modifiers
      ///

      void clear() noexcept { table_.clear(); }

      std::pair<iterator, bool> insert(value_type const& value)
      {
        return table_.insert(value);
      }

      std::pair<iterator, bool> insert(value_type&& value)
      {
        return table_.insert(std::move(value));
      }

      iterator insert(const_iterator, value_type const& value)
      {
        return table_.insert(value).first;
      }

      iterator insert(const_iterator, value_type&& value)
      {
        return table_.insert(std::move(value)).first;
      }

      template <class ForwardIterator>
      typename std::enable_if<
        std::is_base_of<std::forward_iterator_tag, ForwardIterator>::value,
        void>::type
      insert(ForwardIterator first, ForwardIterator last)
      {
        auto const len = std::distance(first, last);
        table_.reserve(len);
        for (auto pos = first; pos != last; ++pos) {
          table_.insert(*pos);
        }
      }

      template <class InputIterator>
      typename std::enable_if<
        !std::is_base_of<std::forward_iterator_tag, InputIterator>::value,
        void>::type
      insert(InputIterator first, InputIterator last)
      {
        for (auto pos = first; pos != last; ++pos) {
          table_.insert(*pos);
        }
      }

      void insert(std::initializer_list<value_type> ilist)
      {
        this->insert(ilist.begin(), ilist.end());
      }

      template <class M>
      std::pair<iterator, bool> insert_or_assign(key_type const& key, M&& obj)
      {
        auto iter_bool_pair = table_.try_emplace(key, std::forward<M>(obj));
        if (iter_bool_pair.second) {
          return iter_bool_pair;
        }
        iter_bool_pair.first->second = std::forward<M>(obj);
        return iter_bool_pair;
      }

      template <class M>
      std::pair<iterator, bool> insert_or_assign(key_type&& key, M&& obj)
      {
        auto iter_bool_pair =
          table_.try_emplace(std::move(key), std::forward<M>(obj));
        if (iter_bool_pair.second) {
          return iter_bool_pair;
        }
        iter_bool_pair.first->second = std::forward<M>(obj);
        return iter_bool_pair;
      }

      template <class M>
      iterator insert_or_assign(const_iterator, key_type const& key, M&& obj)
      {
        return this->insert_or_assign(key, std::forward<M>(obj)).first;
      }

      template <class M>
      iterator insert_or_assign(const_iterator, key_type&& key, M&& obj)
      {
        return this->insert_or_assign(std::move(key), std::forward<M>(obj))
          .first;
      }

      template <class... Args> std::pair<iterator, bool> emplace(Args&&... args)
      {
        return table_.emplace(std::forward<Args>(args)...);
      }

      template <class... Args>
      std::pair<iterator, bool> try_emplace(key_type const& key, Args&&... args)
      {
        return table_.try_emplace(key, std::forward<Args>(args)...);
      }

      template <class... Args>
      std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args)
      {
        return table_.try_emplace(std::move(key), std::forward<Args>(args)...);
      }

      template <class... Args>
      iterator try_emplace(const_iterator, key_type const& key, Args&&... args)
      {
        return table_.try_emplace(key, std::forward<Args>(args)...).first;
      }

      template <class... Args>
      iterator try_emplace(const_iterator, key_type&& key, Args&&... args)
      {
        return table_.try_emplace(std::move(key), std::forward<Args>(args)...)
          .first;
      }

      void erase(iterator pos) { table_.erase(pos); }
      void erase(const_iterator pos) { return table_.erase(pos); }
      size_type erase(key_type const& key) { return table_.erase(key); }

      /// Lookup
      ///

      mapped_type& operator[](key_type const& key)
      {
        return table_.try_emplace(key).first->second;
      }

      mapped_type& operator[](key_type&& key)
      {
        return table_.try_emplace(std::move(key)).first->second;
      }

      size_type count(key_type const& key) const
      {
        auto pos = table_.find(key);
        return pos != table_.end() ? 1 : 0;
      }

      iterator find(key_type const& key) { return table_.find(key); }

      const_iterator find(key_type const& key) const
      {
        return table_.find(key);
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
