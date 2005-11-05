
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

// TODO: This is just the first insert test slightly modified, should do better.

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
            typename Container::value_type::second_type* ref;

            // Looking at TR1 I can't find this requirement anywhere, but it
            // would seem silly not to require it so I think it's either an
            // omission or I haven't looked hard enough.
            STRONG_TEST(tester, x) {
                try {
                    ref = &x[it->first];
                } catch(test::hash_exception) {
                    tester.dismiss();
                    throw;
                }
            }

            DEACTIVATE_EXCEPTIONS;
            *ref = it->second;

            typename Container::iterator pos = x.find(values.get_key(*it));
            BOOST_CHECK(pos != x.end() &&
                    x.key_eq()(values.get_key(*pos), values.get_key(*it)) &&
                    test::equivalent(*it, *pos));
        }
    }
    EXCEPTION_TEST_END
}

typedef boost::unordered_map<int, int> map1;
typedef boost::unordered_map<test::member, test::member, test::hash, test::equals, test::allocator<int> > map2;

AUTO_META_TESTS(
    (insert_individual),
    (map1)(map2)
)
