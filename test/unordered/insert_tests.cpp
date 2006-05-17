
//  Copyright Daniel James 2006. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/next_prior.hpp>
#include "../objects/test.hpp"
#include "../helpers/random_values.hpp"
#include "../helpers/tracker.hpp"
#include "../helpers/equivalent.hpp"
#include "../helpers/invariants.hpp"

#include <iostream>
    
template <class Container>
void unique_insert_tests1(Container* = 0)
{
    std::cerr<<"insert(value) tests for containers with unique keys.\n";

    Container x;
    test::ordered<Container> tracker = test::create_ordered(x);

    test::random_values<Container> v(1000);
    for(typename test::random_values<Container>::iterator it = v.begin();
            it != v.end(); ++it)
    {
        std::pair<typename Container::iterator, bool> r1 = x.insert(*it);
        std::pair<typename test::ordered<Container>::iterator, bool> r2
            = tracker.insert(*it);

        BOOST_TEST(r1.second == r2.second);
        BOOST_TEST(*r1.first == *r2.first);

        tracker.compare_key(x, *it);
    }

    test::check_equivalent_keys(x);
}

template <class Container>
void equivalent_insert_tests1(Container* = 0)
{
    std::cerr<<"insert(value) tests for containers with equivalent keys.\n";

    Container x;
    test::ordered<Container> tracker = test::create_ordered(x);

    test::random_values<Container> v(1000);
    for(typename test::random_values<Container>::iterator it = v.begin();
            it != v.end(); ++it)
    {
        typename Container::iterator r1 = x.insert(*it);
        typename test::ordered<Container>::iterator r2 = tracker.insert(*it);

        BOOST_TEST(*r1 == *r2);

        tracker.compare_key(x, *it);
    }

    test::check_equivalent_keys(x);
}

template <class Container>
void insert_tests2(Container* = 0)
{
    typedef typename test::ordered<Container> tracker_type;
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;
    typedef typename tracker_type::iterator tracker_iterator;

    std::cerr<<"insert(begin(), value) tests.\n";

    {
        Container x;
        tracker_type tracker = test::create_ordered(x);

        test::random_values<Container> v(1000);
        for(typename test::random_values<Container>::iterator it = v.begin();
                it != v.end(); ++it)
        {
            iterator r1 = x.insert(x.begin(), *it);
            tracker_iterator r2 = tracker.insert(tracker.begin(), *it);
            BOOST_TEST(*r1 == *r2);
            tracker.compare_key(x, *it);
        }

        test::check_equivalent_keys(x);
    }

    std::cerr<<"insert(end(), value) tests.\n";

    {
        Container x;
        Container const& x_const = x;
        tracker_type tracker = test::create_ordered(x);

        test::random_values<Container> v(100);
        for(typename test::random_values<Container>::iterator it = v.begin();
                it != v.end(); ++it)
        {
            const_iterator r1 = x.insert(x_const.end(), *it);
            tracker_iterator r2 = tracker.insert(tracker.end(), *it);
            BOOST_TEST(*r1 == *r2);
            tracker.compare_key(x, *it);
        }

        test::check_equivalent_keys(x);
    }

    std::cerr<<"insert(pos, value) tests.\n";

    {
        Container x;
        const_iterator pos = x.begin();
        tracker_type tracker = test::create_ordered(x);

        test::random_values<Container> v(1000);
        for(typename test::random_values<Container>::iterator it = v.begin();
                it != v.end(); ++it)
        {
            pos = x.insert(pos, *it);
            tracker_iterator r2 = tracker.insert(tracker.begin(), *it);
            BOOST_TEST(*pos == *r2);
            tracker.compare_key(x, *it);
        }

        test::check_equivalent_keys(x);
    }

    std::cerr<<"insert single item range tests.\n";

    {
        Container x;
        tracker_type tracker = test::create_ordered(x);

        test::random_values<Container> v(1000);
        for(typename test::random_values<Container>::iterator it = v.begin();
                it != v.end(); ++it)
        {
            x.insert(it, boost::next(it));
            tracker.insert(*it);
            tracker.compare_key(x, *it);
        }

        test::check_equivalent_keys(x);
    }

    std::cerr<<"insert range tests.\n";

    {
        Container x;
        const_iterator pos = x.begin();

        test::random_values<Container> v(1000);
        x.insert(v.begin(), v.end());
        check_container(x, v);

        test::check_equivalent_keys(x);
    }
}

int main()
{
    unique_insert_tests1((boost::unordered_set<int>*) 0);
    equivalent_insert_tests1((boost::unordered_multiset<int>*) 0);
    unique_insert_tests1((boost::unordered_map<int, int>*) 0);
    equivalent_insert_tests1((boost::unordered_multimap<int, int>*) 0);

    unique_insert_tests1((boost::unordered_set<test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);
    equivalent_insert_tests1((boost::unordered_multiset<test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);
    unique_insert_tests1((boost::unordered_map<test::object, test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);
    equivalent_insert_tests1((boost::unordered_multimap<test::object, test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);

    insert_tests2((boost::unordered_set<int>*) 0);
    insert_tests2((boost::unordered_multiset<int>*) 0);
    insert_tests2((boost::unordered_map<int, int>*) 0);
    insert_tests2((boost::unordered_multimap<int, int>*) 0);

    insert_tests2((boost::unordered_set<test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);
    insert_tests2((boost::unordered_multiset<test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);
    insert_tests2((boost::unordered_map<test::object, test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);
    insert_tests2((boost::unordered_multimap<test::object, test::object, test::hash, test::equal_to, test::allocator<test::object> >*) 0);

    return boost::report_errors();
}
