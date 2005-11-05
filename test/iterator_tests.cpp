
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

// TODO: Add these headers to new_iterator_tests.hpp
#include <boost/mpl/and.hpp>
#include <boost/detail/is_incrementable.hpp>
#include <boost/iterator/new_iterator_tests.hpp>

#include <boost/next_prior.hpp>
#include <algorithm>
#include <string>
#include "./helpers/unit_test.hpp"

BOOST_AUTO_UNIT_TEST(iterator_tests)
{
    boost::unordered_set<std::pair<std::string, std::string> > set;
    boost::unordered_multiset<float> multiset;
    boost::unordered_map<int, int> map;
    boost::unordered_multimap<char *, std::pair<int, int> > multimap;
    
    set.insert(std::pair<std::string const, std::string>("Anthony","Cleopatra"));
    set.insert(std::pair<std::string const, std::string>("Victoria","Albert"));
    set.insert(std::pair<std::string const, std::string>("Pete","Dud"));
    set.insert(std::pair<std::string const, std::string>("Blair","Brown"));
    set.insert(std::pair<std::string const, std::string>("John","Yoko"));
    set.insert(std::pair<std::string const, std::string>("Charles","Diana"));
    set.insert(std::pair<std::string const, std::string>("Marx","Engels"));
    set.insert(std::pair<std::string const, std::string>("Sid","Nancy"));
    set.insert(std::pair<std::string const, std::string>("Lucy","Ricky"));
    set.insert(std::pair<std::string const, std::string>("George","Mildred"));
    set.insert(std::pair<std::string const, std::string>("Fanny","Alexander"));
    set.insert(std::pair<std::string const, std::string>("Den","Angie"));
    set.insert(std::pair<std::string const, std::string>("Sonny","Cher"));
    set.insert(std::pair<std::string const, std::string>("Bonnie","Clyde"));
    set.insert(std::pair<std::string const, std::string>("Punch","Judy"));
    set.insert(std::pair<std::string const, std::string>("Powell","Pressburger"));
    set.insert(std::pair<std::string const, std::string>("Jekyll","Hyde"));

    multiset.insert(0.434321);
    multiset.insert(443421);
    multiset.insert(0.434321);
    
    map[98] = 3;
    map[99] = 4;
    map[2000] = 788421;
    map[2001] = 2;
    
    static char* strings1 = "Banjo\0Banjo\0Ukulele";

    static char* strings[] = {
        strings1,
        strings1 + 6,
        strings1 + 12
    };

    BOOST_CHECK(std::string(strings[0]) == std::string(strings[1]));
    BOOST_CHECK(strings[0] != strings[1]);
    
    multimap.insert(std::make_pair(strings[0], std::make_pair(5,6)));
    multimap.insert(std::make_pair(strings[1], std::make_pair(85,32)));
    multimap.insert(std::make_pair(strings[1], std::make_pair(91,142)));
    multimap.insert(std::make_pair(strings[2], std::make_pair(12,3)));
    multimap.insert(std::make_pair(strings[2], std::make_pair(10,94)));
    multimap.insert(std::make_pair(strings[2], std::make_pair(345,42)));
    
    BOOST_CHECK_EQUAL(multimap.count(strings[0]), 1);
    BOOST_CHECK_EQUAL(multimap.count(strings[1]), 2);
    BOOST_CHECK_EQUAL(multimap.count(strings[2]), 3);
    
    boost::forward_readable_iterator_test(set.begin(), set.end(),
        *set.begin(), *boost::next(set.begin()));
    boost::forward_readable_iterator_test(multiset.begin(), multiset.end(),
        *multiset.begin(), *boost::next(multiset.begin()));
    boost::forward_readable_iterator_test(map.begin(), map.end(),
        *map.begin(), *boost::next(map.begin()));
    boost::forward_readable_iterator_test(multimap.begin(), multimap.end(),
        *multimap.begin(), *boost::next(multimap.begin()));
}

BOOST_AUTO_UNIT_TEST(rubbish_iterator_test)
{
    typedef boost::unordered_map<int, int> map;
    typedef map::local_iterator local_iterator;
    typedef map::const_local_iterator const_local_iterator;
    typedef map::iterator iterator;
    typedef map::const_iterator const_iterator;

    map m;
    iterator it;
    const_iterator const_it(it);
    local_iterator local;
    const_local_iterator const_local(local);

    m[10] = 3;
    m[25] = 5;
    m[2] = 21;

    it = m.begin();
    const_it = m.begin();
    map::size_type index = m.bucket(10);
    local = m.begin(index);
    const_local = m.begin(index);

    BOOST_CHECK(it == const_it);
    BOOST_CHECK(const_it == it);
    BOOST_CHECK(local == const_local);
    BOOST_CHECK(const_local == local);

    BOOST_CHECK(it++ == const_it);
    BOOST_CHECK(local++ == const_local);

    BOOST_CHECK(it != const_it);
    BOOST_CHECK(const_it != it);
    BOOST_CHECK(local != const_local);
    BOOST_CHECK(const_local != local);

    BOOST_CHECK(++const_it == it);
    BOOST_CHECK(++const_local == local);

    it = m.begin();
    int values[3];
    std::pair<int const, int> const& r1= *it++;
    values[0] = r1.second;
    values[1] = it++->second;
    values[2] = it++->second;
    BOOST_CHECK(it == m.end());

    std::sort(values, values+3);

    BOOST_CHECK_EQUAL(values[0], 3);
    BOOST_CHECK_EQUAL(values[1], 5);
    BOOST_CHECK_EQUAL(values[2], 21);
}
