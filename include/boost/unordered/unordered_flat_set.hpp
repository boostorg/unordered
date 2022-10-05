// Copyright (C) 2022 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNORDERED_UNORDERED_FLAT_SET_HPP_INCLUDED
#define BOOST_UNORDERED_UNORDERED_FLAT_SET_HPP_INCLUDED

#include <boost/config.hpp>
#if defined(BOOST_HAS_PRAGMA_ONCE)
#pragma once
#endif

#include <boost/unordered/detail/foa.hpp>
#include <boost/unordered/unordered_flat_set_fwd.hpp>

#include <boost/core/allocator_access.hpp>

#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <utility>

namespace boost {
  namespace unordered {
    template <class Key, class Hash, class KeyEqual, class Allocator>
    class unordered_flat_set
    {
      struct set_types
      {
        using key_type = Key;
        using init_type = Key;
        using value_type = Key;
        static Key const& extract(value_type const& key) { return key; }

        template<typename F>
        static void move_parts_to(value_type& x,F f) { f(std::move(x)); }
      };

      using table_type = detail::foa::table<set_types, Hash, KeyEqual,
        typename boost::allocator_rebind<Allocator,
          typename set_types::value_type>::type>;

      table_type table_;

    public:
      using key_type = Key;
      using value_type = typename set_types::value_type;
      using size_type = std::size_t;
      using hasher = Hash;
      using key_equal = KeyEqual;
      using allocator_type = Allocator;
      using reference = value_type&;
      using const_reference = value_type const&;
      using iterator = typename table_type::iterator;
      using const_iterator = typename table_type::const_iterator;

      unordered_flat_set() : unordered_flat_set(0) {}

      explicit unordered_flat_set(size_type n, hasher const& h = hasher(),
        key_equal const& pred = key_equal(),
        allocator_type const& a = allocator_type())
          : table_(n, h, pred, a)
      {
      }

      unordered_flat_set(size_type n, allocator_type const& a)
          : unordered_flat_set(n, hasher(), key_equal(), a)
      {
      }

      explicit unordered_flat_set(allocator_type const& a)
          : unordered_flat_set(0, a)
      {
      }

      template <class Iterator>
      unordered_flat_set(Iterator first, Iterator last, size_type n = 0,
        hasher const& h = hasher(), key_equal const& pred = key_equal(),
        allocator_type const& a = allocator_type())
          : unordered_flat_set(n, h, pred, a)
      {
        this->insert(first, last);
      }

      unordered_flat_set(unordered_flat_set const& other) : table_(other.table_)
      {
      }

      unordered_flat_set(
        unordered_flat_set const& other, allocator_type const& a)
          : table_(other.table_, a)
      {
      }

      unordered_flat_set(unordered_flat_set&& other)
        noexcept(std::is_nothrow_move_constructible<hasher>::value&&
            std::is_nothrow_move_constructible<key_equal>::value&&
              std::is_nothrow_move_constructible<allocator_type>::value)
          : table_(std::move(other.table_))
      {
      }

      unordered_flat_set(unordered_flat_set&& other, allocator_type const& al)
          : table_(std::move(other.table_), al)
      {
      }

      unordered_flat_set(std::initializer_list<value_type> ilist,
        size_type n = 0, hasher const& h = hasher(),
        key_equal const& pred = key_equal(),
        allocator_type const& a = allocator_type())
          : unordered_flat_set(ilist.begin(), ilist.end(), n, h, pred, a)
      {
      }

      ~unordered_flat_set() = default;

      unordered_flat_set& operator=(unordered_flat_set const& other)
      {
        table_ = other.table_;
        return *this;
      }

      unordered_flat_set& operator=(unordered_flat_set&& other) noexcept(
        noexcept(std::declval<table_type&>() = std::declval<table_type&&>()))
      {
        table_ = std::move(other.table_);
        return *this;
      }

      allocator_type get_allocator() const noexcept
      {
        return table_.get_allocator();
      }

      /// Iterators
      ///

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

      template <class InputIterator>
      void insert(InputIterator first, InputIterator last)
      {
        for (auto pos = first; pos != last; ++pos) {
          table_.insert(*pos);
        }
      }

      void insert(std::initializer_list<value_type> ilist)
      {
        this->insert(ilist.begin(), ilist.end());
      }

      template <class... Args> std::pair<iterator, bool> emplace(Args&&... args)
      {
        return table_.emplace(std::forward<Args>(args)...);
      }

      void erase(const_iterator pos) { return table_.erase(pos); }
      iterator erase(const_iterator first, const_iterator last)
      {
        while (first != last) {
          this->erase(first++);
        }
        return iterator{detail::foa::const_iterator_cast_tag{}, last};
      }

      size_type erase(key_type const& key) { return table_.erase(key); }

      /// Lookup
      ///

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

      /// Hash Policy
      ///

      size_type bucket_count() const noexcept { return table_.capacity(); }

      float load_factor() const noexcept { return table_.load_factor(); }

      float max_load_factor() const noexcept
      {
        return table_.max_load_factor();
      }

      void max_load_factor(float) {}

      /// Observers
      ///

      hasher hash_function() const { return table_.hash_function(); }

      key_equal key_eq() const { return table_.key_eq(); }
    };
  } // namespace unordered
} // namespace boost

#endif
