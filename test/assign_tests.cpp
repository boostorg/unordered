
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "./helpers/unit_test.hpp"
#include "./helpers/exception_test.hpp"
#include "./helpers/random_values.hpp"
#include "./helpers/constructors.hpp"
#include "./helpers/constructors.hpp"
#include "./helpers/equivalent.hpp"
#include "./invariant.hpp"

const int num_values = 50;

META_FUNC_TEST_CASE(assign_test1, Container)
{
    test::constructors<Container> constructor;

    test::random_values<Container> values(num_values);
    Container x(values.begin(), values.end(), 0,
            constructor.hasher(55), constructor.key_equal(55),
            constructor.allocator(10));
    x.max_load_factor(0.1);

    EXCEPTION_TEST(10000)
    {
        DEACTIVATE_EXCEPTIONS;
        Container y;
        INVARIANT_CHECK(y);
        ACTIVATE_EXCEPTIONS;

        BOOST_CHECKPOINT("y = x");
        y = x;

        {
            DEACTIVATE_EXCEPTIONS;
            BOOST_CHECK_EQUAL(y.size(), x.size());
            BOOST_CHECK(test::equivalent(y.hash_function(), x.hash_function()));
            BOOST_CHECK(test::equivalent(y.key_eq(), x.key_eq()));
            BOOST_CHECK_EQUAL(y.max_load_factor(), x.max_load_factor());
            BOOST_CHECK(test::equivalent(y.get_allocator(), constructor.allocator()));
            BOOST_CHECK(y.load_factor() <= y.max_load_factor());
            test::check_invariants();
        }

        BOOST_CHECKPOINT("y = y");
        y = y;

        {
            DEACTIVATE_EXCEPTIONS;
            BOOST_CHECK_EQUAL(y.size(), x.size());
            BOOST_CHECK(test::equivalent(y.hash_function(), x.hash_function()));
            BOOST_CHECK(test::equivalent(y.key_eq(), x.key_eq()));
            BOOST_CHECK_EQUAL(y.max_load_factor(), x.max_load_factor());
            BOOST_CHECK(test::equivalent(y.get_allocator(), constructor.allocator()));
            BOOST_CHECK(y.load_factor() <= y.max_load_factor());
            test::check_invariants();
        }

        BOOST_CHECKPOINT("y = Container(values.begin(), values.end())");
        y = Container(values.begin(), values.end());

        {
            DEACTIVATE_EXCEPTIONS;
            BOOST_CHECK_EQUAL(y.size(), x.size());
            BOOST_CHECK(test::equivalent(y.hash_function(), constructor.hasher()));
            BOOST_CHECK(test::equivalent(y.key_eq(), constructor.key_equal()));
            BOOST_CHECK_EQUAL(y.max_load_factor(), 1.0);
            BOOST_CHECK(test::equivalent(y.get_allocator(), constructor.allocator()));
            BOOST_CHECK(y.load_factor() <= 1.0);
            test::check_invariants();
        }
    }
    EXCEPTION_TEST_END
}

AUTO_META_TESTS(
    (assign_test1),
    CONTAINER_SEQ
)
