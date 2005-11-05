
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "./helpers/unit_test.hpp"
#include "./helpers/random_values.hpp"

META_FUNC_TEST_CASE(find_const_test,Container)
{
    test::random_values<Container> values(500);
    typedef typename test::random_values<Container>::iterator iterator;

    Container const x(values.begin(), values.end());

    for(iterator it = values.begin(); it != values.end(); ++it)
    {
        typename Container::const_iterator pos = x.find(values.get_key(*it));
        BOOST_CHECK(pos != x.end());
        BOOST_CHECK(values.get_key(*pos) == values.get_key(*it));
    }
}

META_FUNC_TEST_CASE(find_nonconst_test,Container)
{
    test::random_values<Container> values(500);
    typedef typename test::random_values<Container>::iterator iterator;

    Container x(values.begin(), values.end());

    for(iterator it = values.begin(); it != values.end(); ++it)
    {
        typename Container::iterator pos = x.find(values.get_key(*it));
        BOOST_CHECK(pos != x.end());
        BOOST_CHECK(values.get_key(*pos) == values.get_key(*it));
    }
}

META_FUNC_TEST_CASE(missing_test,Container)
{
    test::random_values<Container> values(10);
    Container x(values.begin(), values.end());

    typedef typename test::random_values<Container>::value_type value_type;
    test::generator<value_type> generator;

    for(int i = 0; i < 500; ++i)
    {
        value_type const value = generator();
        bool const present_in_values = values.find(value) != values.end();
        bool const present_in_container = x.find(values.get_key(value))
                != x.end();
        BOOST_CHECK(present_in_values == present_in_container);
    }
}

META_FUNC_TEST_CASE(empty_test,Container)
{
    typedef test::random_values<Container> random_values;
    typedef typename random_values::value_type value_type;
    test::generator<value_type> generator;
    Container x;

    for(int i = 0; i < 500; ++i)
    {
        BOOST_CHECK(x.find(random_values::get_key(generator())) == x.end());
    }
}

AUTO_META_TESTS(
    (find_const_test)(find_nonconst_test)(missing_test)(empty_test),
    CONTAINER_SEQ
)
