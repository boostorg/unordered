
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "./helpers/unit_test.hpp"
#include "./helpers/exception_test.hpp"
#include "./helpers/random_values.hpp"
#include "./helpers/input_iterator_adaptor.hpp"
#include "./helpers/constructors.hpp"
#include "./helpers/equivalent.hpp"
#include "./invariant.hpp"

META_FUNC_TEST_CASE(empty_construct_test1, Container)
{
    test::constructors<Container> constructor;

    EXCEPTION_TEST(1000)
    {
        // TR1 6.3.1/9 row 4
        Container x(100, constructor.hasher(55), constructor.key_equal(55));
        BOOST_CHECK(x.bucket_count() >= 100);
        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher(55)));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal(55)));

        // TODO: Where?
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator()));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(empty_construct_test2, Container)
{
    test::constructors<Container> constructor;

    EXCEPTION_TEST(1000)
    {
        // TR1 6.3.1/9 row 5

        // I can only use the default hasher here - as it'll match the default
        // key_equal.
        Container x(100, constructor.hasher());
        BOOST_CHECK(x.empty());
        BOOST_CHECK(x.bucket_count() >= 100);
        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher()));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal()));

        // TODO: Where?
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator()));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(empty_construct_test3, Container)
{
    test::constructors<Container> constructor;

    EXCEPTION_TEST(1000)
    {
        // TR1 6.3.1/9 row 6

        Container x(200);
        BOOST_CHECK(x.empty());
        BOOST_CHECK(x.bucket_count() >= 200);
        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher()));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal()));

        // TODO: Where?
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator()));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(empty_construct_test4, Container)
{
    test::constructors<Container> constructor;

    EXCEPTION_TEST(1000)
    {
        // TR1 6.3.1/9 row 7

        Container x;
        BOOST_CHECK(x.empty());
        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher()));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal()));

        // TODO: Where?
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator()));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(empty_construct_test5, Container)
{
    test::constructors<Container> constructor;

    EXCEPTION_TEST(1000)
    {
        // TODO: Where?
        Container x(100, constructor.hasher(55), constructor.key_equal(55), constructor.allocator(10));
        BOOST_CHECK(x.empty());
        BOOST_CHECK(x.bucket_count() >= 100);
        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher(55)));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal(55)));
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator(10)));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(range_construct_test1, Container)
{
    test::constructors<Container> constructor;
    test::random_values<Container> values(10);

    EXCEPTION_TEST(1000)
    {
        // TR1 6.3.1/9 row 8

        Container x(values.begin(), values.end(), 100,
                constructor.hasher(55), constructor.key_equal(55));
        BOOST_CHECK(x.bucket_count() >= 100);
        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher(55)));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal(55)));
        // TODO: Check that values are in container.

        // TODO: Where?
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator()));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(range_construct_test2, Container)
{
    test::constructors<Container> constructor;
    test::random_values<Container> values(10);

    EXCEPTION_TEST(1000)
    {
        // TR1 6.3.1/9 row 9

        Container x(values.begin(), values.end(), 100, constructor.hasher());
        BOOST_CHECK(x.bucket_count() >= 100);
        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher()));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal()));
        // TODO: Check that values are in container.

        // TODO: Where?
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator()));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(range_construct_test3, Container)
{
    test::constructors<Container> constructor;
    test::random_values<Container> values(20);

    EXCEPTION_TEST(1000)
    {
        // TR1 6.3.1/9 row 10

        Container x(values.begin(), values.end(), 10);
        BOOST_CHECK(x.bucket_count() >= 10);
        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher()));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal()));
        // TODO: Check that values are in container.

        // TODO: Where?
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator()));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(range_construct_test4, Container)
{
    test::constructors<Container> constructor;
    test::random_values<Container> values(20);

    EXCEPTION_TEST(1000)
    {
        // TR1 6.3.1/9 row 11

        Container x(values.begin(), values.end());
        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher()));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal()));
        // TODO: Check that values are in container.

        // TODO: Where?
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator()));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(range_construct_test5, Container)
{
    test::constructors<Container> constructor;
    test::random_values<Container> values(10);

    EXCEPTION_TEST(1000)
    {
        // TODO: Where?
        Container x(values.begin(), values.end(), 10,
                constructor.hasher(55), constructor.key_equal(55), constructor.allocator(10));
        BOOST_CHECK(x.bucket_count() >= 10);
        BOOST_CHECK(test::equivalent(x.hash_function(), constructor.hasher(55)));
        BOOST_CHECK(test::equivalent(x.key_eq(), constructor.key_equal(55)));
        BOOST_CHECK(test::equivalent(x.get_allocator(), constructor.allocator(10)));
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END
}

// TODO: I should probably just make all the tests run from an input iterator.
META_FUNC_TEST_CASE(input_iterator_construct_test1, Container)
{
    test::random_values<Container> values(10);

    EXCEPTION_TEST(1000)
    {
        Container x(
                test::make_input_iterator(values.begin()),
                test::make_input_iterator(values.end())
                );
        BOOST_CHECK_EQUAL(x.max_load_factor(), 1.0);

        test::invariant_check(x);
    }
    EXCEPTION_TEST_END

}

AUTO_META_TESTS(
    (empty_construct_test1)(empty_construct_test2)(empty_construct_test3)
    (empty_construct_test4)(empty_construct_test5)
    (range_construct_test1)(range_construct_test2)(range_construct_test3)
    (range_construct_test4)(range_construct_test5)
    (input_iterator_construct_test1),
    CONTAINER_SEQ
)
