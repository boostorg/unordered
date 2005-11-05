
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TESTS_CONTAINERS_HEADER)
#define BOOST_UNORDERED_TESTS_CONTAINERS_HEADER

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/type_traits/broken_compiler_spec.hpp>
#include <string>
#include "./helpers/allocator.hpp"
#include "./helpers/functional.hpp"
#include "./helpers/member.hpp"

typedef boost::unordered_set<
        test::member
    > test_set;
typedef boost::unordered_multiset<
        test::member, test::hash, test::equals,
        test::allocator<test::member>
    > test_multiset;
typedef boost::unordered_map<
        test::member, test::member, test::hash, test::equals,
        test::allocator<std::pair<test::member const, test::member> >
    > test_map;
typedef boost::unordered_multimap<
        test::member, test::member, test::hash, test::equals,
        test::minimal_allocator<std::pair<test::member const, test::member> >
    > test_multimap;

typedef boost::unordered_set<
        int, test::hash, test::equals,
        test::allocator<int>
    > set_int;
typedef boost::unordered_multiset<
        std::string, test::hash, test::equals,
        test::allocator<std::string>
    > multiset_string;
typedef boost::unordered_map<
        test::member, std::string, test::hash, test::equals,
        test::allocator<std::pair<test::member const, std::string> >
    > map_member_string;
typedef boost::unordered_multimap<
        int, test::member, test::hash, test::equals,
        test::allocator<std::pair<int const, test::member> >
    > multimap_int_member;
typedef boost::unordered_map<
        char, test::member, test::hash, test::equals,
        test::allocator<std::pair<char const, test::member> >
    > map_char_member;
typedef boost::unordered_multiset<
        char, test::hash, test::equals,
        test::allocator<char>
    > multiset_char;

typedef std::pair<test::member, std::string> pair1;
typedef std::pair<int, test::member> pair2;
typedef std::pair<char, test::member> pair3;
BOOST_TT_BROKEN_COMPILER_SPEC(std::string)
BOOST_TT_BROKEN_COMPILER_SPEC(pair1)
BOOST_TT_BROKEN_COMPILER_SPEC(pair2)
BOOST_TT_BROKEN_COMPILER_SPEC(pair3)
BOOST_TT_BROKEN_COMPILER_SPEC(test::member)

#ifdef REDUCED_TESTS
#define CONTAINER_SEQ \
        (test_set)(test_multiset)(test_map)(test_multimap)
#else
#define CONTAINER_SEQ \
        (set_int)(multiset_string)(map_member_string) \
        (multimap_int_member)(map_char_member)(multiset_char)
#endif

#endif
