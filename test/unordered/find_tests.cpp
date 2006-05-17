
//  Copyright Daniel James 2006. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/detail/lightweight_test.hpp>
#include "../objects/test.hpp"
#include "../helpers/random_values.hpp"
#include "../helpers/tracker.hpp"
#include "../helpers/helpers.hpp"

template <class X>
void find_tests1(X*)
{
    {
        test::random_values<X> v(500);
        X x(v.begin(), v.end());
        X const& x_const = x;
        test::ordered<X> tracker = test::create_ordered(x);
        tracker.insert(v.begin(), v.end());

        for(typename test::ordered<X>::const_iterator it =
                tracker.begin(); it != tracker.end(); ++it)
        {
            typename X::key_type key = test::get_key<X>(*it);
            typename X::iterator pos = x.find(key);
            typename X::const_iterator const_pos = x_const.find(key);
            BOOST_TEST(pos != x.end() &&
                    x.key_eq()(key, test::get_key<X>(*pos)));
            BOOST_TEST(const_pos != x_const.end() &&
                    x_const.key_eq()(key, test::get_key<X>(*const_pos)));

            BOOST_TEST(x.count(key) == tracker.count(key));

            test::compare_pairs(x.equal_range(key),
                    tracker.equal_range(key),
                    (typename test::non_const_value_type<X>::type*) 0);
            test::compare_pairs(x_const.equal_range(key),
                    tracker.equal_range(key),
                    (typename test::non_const_value_type<X>::type*) 0);
        }

        test::random_values<X> v2(500);
        for(typename test::random_values<X>::const_iterator it =
                v2.begin(); it != v2.end(); ++it)
        {
            typename X::key_type key = test::get_key<X>(*it);
            if(tracker.find(test::get_key<X>(key)) == tracker.end())
            {
                BOOST_TEST(x.find(key) == x.end());
                BOOST_TEST(x_const.find(key) == x_const.end());
                BOOST_TEST(x.count(key) == 0);
                std::pair<typename X::iterator,
                    typename X::iterator> range = x.equal_range(key);
                BOOST_TEST(range.first == range.second);
            }
        }
    }

    {
        X x;

        test::random_values<X> v2(5);
        for(typename test::random_values<X>::const_iterator it =
                v2.begin(); it != v2.end(); ++it)
        {
            typename X::key_type key = test::get_key<X>(*it);
            BOOST_TEST(x.find(key) == x.end());
            BOOST_TEST(x.count(key) == 0);
            std::pair<typename X::iterator,
                typename X::iterator> range = x.equal_range(key);
            BOOST_TEST(range.first == range.second);
        }
    }
}

int main()
{
    find_tests1((boost::unordered_set<int>*) 0);
    find_tests1((boost::unordered_multiset<int>*) 0);
    find_tests1((boost::unordered_map<int, int>*) 0);
    find_tests1((boost::unordered_multimap<int, int>*) 0);

    find_tests1((boost::unordered_set<test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);
    find_tests1((boost::unordered_multiset<test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);
    find_tests1((boost::unordered_map<test::object, test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);
    find_tests1((boost::unordered_multimap<test::object, test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);
}
