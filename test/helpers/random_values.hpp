
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_RANDOM_VALUES_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_RANDOM_VALUES_HEADER

#include "./generators.hpp"
#include "./metafunctions.hpp"
#include <vector>
#include <algorithm>
#include <iterator>

namespace test
{
    template <class Container>
    struct accessors
    {
        // get_key
        //
        // Given either the value_type or the key_type returns the key.
        static typename Container::key_type const& 
            get_key(typename Container::key_type const& x)
        {
            return x;
        }

        template <class K, class M>
        static typename Container::key_type const&
            get_key(std::pair<K, M> const& x)
        {
            return x.first;
        }

        static typename Container::value_type const& 
            get_mapped(typename Container::key_type const& x)
        {
            return x;
        }

        template <class K, class M>
        static M const&
            get_mapped(std::pair<K, M> const& x)
        {
            return x.second;
        }
    };

    template <class Container>
    struct random_values : public accessors<Container>
    {
        typedef accessors<Container> base;

        typedef typename non_const_value_type<Container>::type value_type;
        typedef typename Container::key_type key_type;

        std::vector<value_type> values_;
        typedef typename std::vector<value_type>::iterator iterator;
        typedef typename std::vector<value_type>::const_iterator const_iterator;

        explicit random_values(std::size_t count)
        {
            values_.reserve(count);
            std::generate_n(std::back_inserter(values_),
                    count, test::generator<value_type>());
        }

        iterator begin() { return values_.begin(); }
        iterator end() { return values_.end(); }
        const_iterator begin() const { return values_.begin(); }
        const_iterator end() const { return values_.end(); }

        value_type const& operator[](std::size_t i) const { return values_[i]; }

        struct key_matcher0
        {
            template <class X, class Y>
            bool operator()(X const& x, Y const& y) const
            {
                return base::get_key(x) == base::get_key(y);
            }
        };

        // No, I don't know why didn't I just use bind.
        struct key_matcher1
        {
            key_type x;

            key_matcher1(key_type const& x) : x(x) {}

            bool operator()(key_type const& y)
            {
                return x == y;
            }

            template <class M>
            bool operator()(std::pair<key_type, M> const& y)
            {
                return x == y.first;
            }
        };

        static key_matcher0 key_match()
        {
            return key_matcher0();
        }

        static key_matcher1 key_match(key_type const& x)
        {
            return key_matcher1(x);
        }

        template <class M>
        static key_matcher1 key_match(std::pair<key_type, M> const& x)
        {
            return key_matcher1(x.first);
        }

        template <class K>
        iterator find(K const& x)
        {
            return std::find_if(values_.begin(), values_.end(), key_match(x));
        }

        template <class K>
        std::size_t count(K const& x)
        {
            return std::count_if(values_.begin(), values_.end(),
                    key_match(x));
        }

        template <class K>
        std::size_t key_count(K const& x)
        {
            if(has_unique_keys<Container>::value)
                return find(x) != values_.end();
            else
                return count(x);
        }

        static bool is_unique()
        {
            return has_unique_keys<Container>::value;
        }
    };

    template <class Container>
    struct sorted_random_values : public random_values<Container>
    {
        typedef random_values<Container> base;
        typedef typename base::value_type value_type;
        typedef typename base::key_type key_type;
        typedef typename base::iterator iterator;
        typedef typename base::const_iterator const_iterator;

        explicit sorted_random_values(std::size_t count)
            : base(count)
        {
            std::stable_sort(this->begin(), this->end());
        }

        struct key_compare0
        {
            template <class X, class Y>
            bool operator()(X const& x, Y const& y) const
            {
                return base::get_key(x) < base::get_key(y);
            }
        };

        static key_compare0 key_compare()
        {
            return key_compare0();
        }

        template <class K>
        iterator find(K const& x)
        {
            iterator pos = std::lower_bound(this->begin(), this->end(), x, key_compare());
            return this->key_match()(x, *pos) ? pos : this->end();
        }

        template <class K>
        std::size_t count(K const& x)
        {
            std::pair<iterator, iterator> range =
                std::equal_range(this->begin(), this->end(), x, key_compare());
            return range.second - range.first;
        }

        template <class K>
        std::size_t key_count(K const& x)
        {
            if(has_unique_keys<Container>::value)
                return find(x) != this->end();
            else
                return count(x);
        }
    };
}

#endif
