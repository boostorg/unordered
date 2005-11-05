
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_EXCEPTION_TEST_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_EXCEPTION_TEST_HEADER

#include <boost/preprocessor/cat.hpp>
#include <exception>
#include "./exception.hpp"
#include "./exception_trigger.hpp"
#include "./base.hpp"

namespace test
{
    void exception_start(int);
    bool exception_loop_test();
    void exception_loop();
    void exception_failure();

    bool true_once();
}

#define EXCEPTION_TEST(count) \
    for(::test::exception_start(count); ::test::exception_loop_test(); \
            ::test::exception_loop()) \
    try

#define EXCEPTION_TEST_END \
    catch(::test::exception const&) { ::test::exception_failure(); }

#endif
