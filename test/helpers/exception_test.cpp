
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./exception_test.hpp"
#include <boost/test/test_tools.hpp>
#include <cassert>
#include <cstdlib>

namespace test
{
    // TODO: (Writing this here instead of the headers to avoid recompiling
    // the world)
    //
    // There are some major design flaws with the exception testing code,
    // apart from global variables that is.

    namespace
    {
        int num_iterations = 0;
        int current_iteration = 0;
        int trigger_count = 0;
        int max_trigger_count = 0;
        bool failed = false;
        bool exception_testing = false;
        bool exceptions_active = false;
    }

    void exception_start(int n)
    {
        num_iterations = n;
        current_iteration = 0;
        max_trigger_count = 0;
        trigger_count = 0;
        failed = false;
        exception_testing = true;
        exceptions_active = true;
    }

    void exception_loop()
    {
        BOOST_CHECK(exceptions_active);

        ++current_iteration;
        max_trigger_count = trigger_count;
        exception_testing = failed;
        exceptions_active = failed;
        trigger_count = 0;
        failed = false;
    }

    bool exception_loop_test()
    {
        if(exception_testing && current_iteration == num_iterations) {
            BOOST_ERROR("Too many iterations");
            return false;
        }
        else {
            return exception_testing;
        }
    }

    void exception_failure()
    {
        failed = true;
    }

    bool true_once()
    {
        ++trigger_count;
        return !exception_testing || trigger_count > max_trigger_count;
    }

    bool exception_trigger_test()
    {
        ++trigger_count;
        return exception_testing && exceptions_active
            && trigger_count > max_trigger_count;
    }

    void exception_trigger()
    {
        if(exception_trigger_test()) throw exception();
    }

    void exception_trigger(char const* message)
    {
        if(exception_trigger_test()) throw exception(message);
    }

    bool exceptions_activate(bool value)
    {
        bool old = exceptions_active;
        exceptions_active = value;
        return old;
    }

    exception_control::exception_control(bool value)
        : old_value(exceptions_activate(value))
    {
    }

    exception_control::~exception_control()
    {
        exceptions_activate(old_value);
    }
}
