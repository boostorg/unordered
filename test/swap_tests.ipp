
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <stdexcept>
#include "./helpers/unit_test.hpp"
#include "./helpers/exception_test.hpp"
#include "./helpers/random_values.hpp"
#include "./helpers/constructors.hpp"
#include "./helpers/equivalent.hpp"
#include "./invariant.hpp"

META_FUNC_TEST_CASE(swap_test1, Container)
{
    test::constructors<Container> constructor;

    test::random_values<Container> values_x(10);
    test::random_values<Container> values_y(10);

    EXCEPTION_TEST(1000)
    {
        DEACTIVATE_EXCEPTIONS;

        Container x(values_x.begin(), values_x.end(), 0,
            constructor.hasher(55), constructor.key_equal(55), constructor.allocator(10));
        x.max_load_factor(0.5);
        Container y(values_y.begin(), values_y.end(), 0,
            constructor.hasher(23), constructor.key_equal(23), constructor.allocator(12));
        y.max_load_factor(2.0);

        INVARIANT_CHECK(x);
        INVARIANT_CHECK(y);

        ACTIVATE_EXCEPTIONS;

        if(BOOST_UNORDERED_SWAP_METHOD == 1
                && !test::equivalent(x.get_allocator(), y.get_allocator())) {
            BOOST_CHECK_THROW(x.swap(y), std::runtime_error);
        }
        else {
            x.swap(y);

            BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher(23)));
            BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal(23)));
            BOOST_CHECK_EQUAL(x.max_load_factor(), 2.0);
#if BOOST_UNORDERED_SWAP_METHOD == 2
            BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator(10)));
#else
            BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator(12)));
#endif

            BOOST_CHECK(test::equivalent(y.hash_function(), constructor.hasher(55)));
            BOOST_CHECK(test::equivalent(y.key_eq(), constructor.key_equal(55)));
            BOOST_CHECK_EQUAL(y.max_load_factor(), 0.5);
#if BOOST_UNORDERED_SWAP_METHOD == 2
            BOOST_CHECK(test::equivalent(y.get_allocator(), constructor.allocator(12)));
#else
            BOOST_CHECK(test::equivalent(y.get_allocator(), constructor.allocator(10)));
#endif
        }
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(self_swap, Container)
{
    test::constructors<Container> constructor;
    test::random_values<Container> values_x(10);

    EXCEPTION_TEST(1000)
    {
        DEACTIVATE_EXCEPTIONS;

        Container x(values_x.begin(), values_x.end(), 0,
            constructor.hasher(55), constructor.key_equal(55),
            constructor.allocator(10));
        x.max_load_factor(0.5);

        INVARIANT_CHECK(x);
        
        ACTIVATE_EXCEPTIONS;

        x.swap(x);

        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher(55)));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal(55)));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 0.5);
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator(10)));
    }
    EXCEPTION_TEST_END
}

AUTO_META_TESTS(
    (swap_test1)(self_swap),
    CONTAINER_SEQ
)
