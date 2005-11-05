
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <boost/next_prior.hpp>
#include "./helpers/unit_test.hpp"
#include "./helpers/exception_test.hpp"
#include "./helpers/random_values.hpp"
#include "./helpers/input_iterator_adaptor.hpp"
#include "./strong.hpp"
#include "./invariant.hpp"

META_FUNC_TEST_CASE(insert_individual,Container)
{
    test::random_values<Container> values(100);

    EXCEPTION_TEST(1000)
    {
        DEACTIVATE_EXCEPTIONS;
        Container x;

        INVARIANT_CHECK(x);

        ACTIVATE_EXCEPTIONS;

        for(typename test::random_values<Container>::iterator
                it = values.begin(); it != values.end(); ++it)
        {
            STRONG_TEST(tester, x) {
                try {
                    x.insert(*it);
                } catch(test::hash_exception) {
                    tester.dismiss();
                    throw;
                }
            }

            DEACTIVATE_EXCEPTIONS;
            typename Container::iterator pos = x.find(values.get_key(*it));
            BOOST_CHECK(pos != x.end() &&
                    x.key_eq()(values.get_key(*pos), values.get_key(*it)));
        }
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(insert_with_previous_item_hint,Container)
{
    test::random_values<Container> values(10);

    EXCEPTION_TEST(1000)
    {
        DEACTIVATE_EXCEPTIONS;

        Container x;
        INVARIANT_CHECK(x);

        ACTIVATE_EXCEPTIONS;

        typename Container::const_iterator prev = x.begin();

        for(typename test::random_values<Container>::iterator
                it = values.begin(); it != values.end(); ++it)
        {
            x.insert(prev, *it);

            typename Container::iterator pos = x.find(values.get_key(*it));
            BOOST_CHECK(pos != x.end() &&
                    x.key_eq()(values.get_key(*pos), values.get_key(*it)));
            prev = pos;
        }
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(insert_with_begin_hint,Container)
{
    test::random_values<Container> values(10);

    EXCEPTION_TEST(1000)
    {
        DEACTIVATE_EXCEPTIONS;
        Container x;
        INVARIANT_CHECK(x);
        ACTIVATE_EXCEPTIONS;

        for(typename test::random_values<Container>::iterator
                it = values.begin(); it != values.end(); ++it)
        {
            x.insert(x.begin(), *it);

            typename Container::iterator pos = x.find(values.get_key(*it));
            BOOST_CHECK(pos != x.end() &&
                    x.key_eq()(values.get_key(*pos), values.get_key(*it)));
        }
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(insert_with_end_hint,Container)
{
    test::random_values<Container> values(10);

    EXCEPTION_TEST(1000)
    {
        DEACTIVATE_EXCEPTIONS;
        Container x;
        INVARIANT_CHECK(x);
        ACTIVATE_EXCEPTIONS;

        for(typename test::random_values<Container>::iterator
                it = values.begin(); it != values.end(); ++it)
        {
            x.insert(x.end(), *it);

            typename Container::iterator pos = x.find(values.get_key(*it));
            BOOST_CHECK(pos != x.end() &&
                    x.key_eq()(values.get_key(*pos), values.get_key(*it)));
        }
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(insert_with_random_hint,Container)
{
    test::random_values<Container> values(10);

    EXCEPTION_TEST(1000)
    {
        DEACTIVATE_EXCEPTIONS;
        Container x;
        INVARIANT_CHECK(x);
        ACTIVATE_EXCEPTIONS;

        for(typename test::random_values<Container>::iterator
                it = values.begin(); it != values.end(); ++it)
        {
            using namespace std;
            x.insert(boost::next(x.begin(), rand() % (x.size() + 1)), *it);

            typename Container::iterator pos = x.find(values.get_key(*it));
            BOOST_CHECK(pos != x.end() &&
                    x.key_eq()(values.get_key(*pos), values.get_key(*it)));
        }
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(insert_range,Container)
{
    test::random_values<Container> values(10);

    EXCEPTION_TEST(1000)
    {
        DEACTIVATE_EXCEPTIONS;
        Container x;
        INVARIANT_CHECK(x);
        ACTIVATE_EXCEPTIONS;

        x.insert(values.begin(), values.end());
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(insert_range_input_iterator,Container)
{
    test::random_values<Container> values(10);

    EXCEPTION_TEST(1000)
    {
        DEACTIVATE_EXCEPTIONS;
        Container x;
        INVARIANT_CHECK(x);
        ACTIVATE_EXCEPTIONS;

        x.insert(test::make_input_iterator(values.begin()),
                test::make_input_iterator(values.end()));
    }
    EXCEPTION_TEST_END
}

AUTO_META_TESTS(
    (insert_individual)(insert_with_previous_item_hint)(insert_with_begin_hint)
    (insert_with_end_hint)(insert_with_random_hint)(insert_range)
    (insert_range_input_iterator),
    CONTAINER_SEQ
)
