#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "../objects/exception.hpp"

typedef boost::unordered_set<
    test::exception::object,
    test::exception::hash,
    test::exception::equal_to,
    test::exception::allocator<test::exception::object> > set;
typedef boost::unordered_multiset<
    test::exception::object,
    test::exception::hash,
    test::exception::equal_to,
    test::exception::allocator<test::exception::object> > multiset;
typedef boost::unordered_map<
    test::exception::object,
    test::exception::object,
    test::exception::hash,
    test::exception::equal_to,
    test::exception::allocator<test::exception::object> > map;
typedef boost::unordered_multimap<
    test::exception::object,
    test::exception::object,
    test::exception::hash,
    test::exception::equal_to,
    test::exception::allocator<test::exception::object> > multimap;

#define CONTAINER_SEQ (set)(multiset)(map)(multimap)
