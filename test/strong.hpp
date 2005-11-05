
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TESTS_STRONG_HEADER)
#define BOOST_UNORDERED_TESTS_STRONG_HEADER

#include "./helpers/strong.hpp"
#include "./helpers/equivalent.hpp"
#include "./helpers/metafunctions.hpp"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <deque>
#include <algorithm>

namespace test
{
    struct equals_t
    {
        template <class X, class Y>
        bool operator()(X const& x, Y const& y)
        {
            return x == y;
        }

        template <class X1, class X2, class Y1, class Y2>
        bool operator()(std::pair<X1, X2> const& x, std::pair<Y1, Y2> const& y)
        {
            return x.first == y.first && x.second == y.second;
        }
    } equals1;

    template <class Container>
    class unordered_strong_tester
        : public strong_tester
    {
        Container const& reference_;

        typename Container::size_type size_;
        typename Container::hasher hasher_;
        typename Container::key_equal key_equal_;

        std::deque<typename non_const_value_type<Container>::type> values_;
    public:
        unordered_strong_tester(Container const &x)
            : reference_(x), size_(x.size()),
            hasher_(x.hash_function()), key_equal_(x.key_eq()),
            values_(x.begin(), x.end())
        {
        }

        void test()
        {
            BOOST_CHECK(size_ == reference_.size());
            BOOST_CHECK(test::equivalent(hasher_, reference_.hash_function()));
            BOOST_CHECK(test::equivalent(key_equal_, reference_.key_eq()));
            BOOST_CHECK(values_.size() == reference_.size());
            BOOST_CHECK(values_.size() == reference_.size() &&
                    std::equal(values_.begin(), values_.end(), reference_.begin(), equals1));
        }
    private:
        unordered_strong_tester();
    };
}

namespace boost
{
    template <class T, class Hash, class Pred, class Alloc>
    test::strong_tester_ptr create_tester_impl(
        boost::unordered_set<T, Hash, Pred, Alloc> const& x, int)
    {
        return test::strong_tester_ptr(new test::unordered_strong_tester<
                boost::unordered_set<T, Hash, Pred, Alloc> >(x));
    }

    template <class T, class Hash, class Pred, class Alloc>
    test::strong_tester_ptr create_tester_impl(
        boost::unordered_multiset<T, Hash, Pred, Alloc> const& x, int)
    {
        return test::strong_tester_ptr(new test::unordered_strong_tester<
                boost::unordered_multiset<T, Hash, Pred, Alloc> >(x));
    }

    template <class K, class T, class Hash, class Pred, class Alloc>
    test::strong_tester_ptr create_tester_impl(
        boost::unordered_map<K, T, Hash, Pred, Alloc> const& x, int)
    {
        return test::strong_tester_ptr(new test::unordered_strong_tester<
                boost::unordered_map<K, T, Hash, Pred, Alloc> >(x));
    }

    template <class K, class T, class Hash, class Pred, class Alloc>
    test::strong_tester_ptr create_tester_impl(
        boost::unordered_multimap<K, T, Hash, Pred, Alloc> const& x, int)
    {
        return test::strong_tester_ptr(new test::unordered_strong_tester<
                boost::unordered_multimap<K, T, Hash, Pred, Alloc> >(x));
    }
}

#endif
