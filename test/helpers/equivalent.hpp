
//  Copyright Daniel James 2005-2006. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TESTS_EQUIVALENT_HEADER)
#define BOOST_UNORDERED_TESTS_EQUIVALENT_HEADER

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <vector>
#include <algorithm>
#include "./metafunctions.hpp"

namespace test
{
    template <class T>
    bool equivalent(T const& x, T const& y) {
        return x == y;
    }

    template <class T>
    bool equivalent(boost::hash<T> const&, boost::hash<T> const&) {
        return true;
    }

    template <class T>
    bool equivalent(std::equal_to<T> const&, std::equal_to<T> const&) {
        return true;
    }

    template <class Container>
    class unordered_equivalence_tester
    {
        typename Container::size_type size_;
        typename Container::hasher hasher_;
        typename Container::key_equal key_equal_;
        float max_load_factor_;

        typedef typename non_const_value_type<Container>::type value_type;
        std::vector<value_type> values_;
    public:
        unordered_equivalence_tester(Container const &x)
            : size_(x.size()),
            hasher_(x.hash_function()), key_equal_(x.key_eq()),
            max_load_factor_(x.max_load_factor()),
            values_()
        {
            // Can't initialise values_ straight from x because of Visual C++ 6
            values_.reserve(x.size());
            std::copy(x.begin(), x.end(), std::back_inserter(values_));
            
            std::sort(values_.begin(), values_.end());
        }

        bool operator()(Container const& x) const
        {
            if(!((size_ == x.size()) &&
                (test::equivalent(hasher_, x.hash_function())) &&
                (test::equivalent(key_equal_, x.key_eq())) &&
                (max_load_factor_ == x.max_load_factor()) &&
                (values_.size() == x.size()))) return false;

            std::vector<value_type> copy;
            copy.reserve(x.size());
            std::copy(x.begin(), x.end(), std::back_inserter(copy));
            std::sort(copy.begin(), copy.end());
            return(std::equal(values_.begin(), values_.end(), copy.begin()));
        }
    private:
        unordered_equivalence_tester();
    };
}

#endif
