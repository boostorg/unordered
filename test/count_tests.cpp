
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "./helpers/unit_test.hpp"
#include "./helpers/random_values.hpp"

META_FUNC_TEST_CASE(count_const_test, Container)
{
    test::random_values<Container> values(500);
    typedef typename test::random_values<Container>::iterator iterator;

    Container const x(values.begin(), values.end());

    for(iterator it = values.begin(); it != values.end(); ++it)
    {
        BOOST_CHECK_EQUAL(x.count(values.get_key(*it)), values.key_count(*it));
    }

    typedef typename test::random_values<Container>::value_type value_type;
    test::generator<value_type> generator;

    for(int i = 0; i < 500; ++i)
    {
        value_type value = generator();
        BOOST_CHECK_EQUAL(x.count(values.get_key(value)), values.key_count(value));
    }
}

META_FUNC_TEST_CASE(count_nonconst_test, Container)
{
    test::random_values<Container> values(500);
    typedef typename test::random_values<Container>::iterator iterator;

    Container x(values.begin(), values.end());

    for(iterator it = values.begin(); it != values.end(); ++it)
    {
        BOOST_CHECK_EQUAL(x.count(values.get_key(*it)), values.key_count(*it));
    }
}

META_FUNC_TEST_CASE(empty_test, Container)
{
    typedef test::random_values<Container> random_values;
    typedef typename random_values::value_type value_type;
    test::generator<value_type> generator;
    Container x;

    for(int i = 0; i < 500; ++i)
    {
        BOOST_CHECK_EQUAL(x.count(random_values::get_key(generator())), 0u);
    }
}

AUTO_META_TESTS(
    (count_const_test)(count_nonconst_test)(empty_test),
    CONTAINER_SEQ
)
