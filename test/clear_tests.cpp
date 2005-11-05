
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "./helpers/unit_test.hpp"
#include "./helpers/random_values.hpp"
#include "./helpers/constructors.hpp"
#include "./invariant.hpp"

META_FUNC_TEST_CASE(clear_test, Container)
{
    test::constructors<Container> constructor;
    test::random_values<Container> values(100);
    Container x(values.begin(), values.end(), 0,
            constructor.hasher(55), constructor.key_equal(55),
            constructor.allocator(10));

    x.clear();
    BOOST_CHECK(x.empty());
    test::invariant_check(x);
}

AUTO_META_TESTS(
    (clear_test),
    CONTAINER_SEQ
)
