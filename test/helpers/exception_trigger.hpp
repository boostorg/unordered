
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_EXCEPTION_TRIGGER_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_EXCEPTION_TRIGGER_HEADER

#include <boost/preprocessor/cat.hpp>

namespace test
{
    // Exception Handling

    bool exception_trigger_test();
    void exception_trigger();
    void exception_trigger(char const*);

    template <class T>
    void exception_trigger(T*)
    {
        if(exception_trigger_test()) throw T();
    }

    template <class T>
    void exception_trigger(T*, char const* msg)
    {
        if(exception_trigger_test()) throw T(msg);
    }

    struct exception_control
    {
        bool old_value;

        exception_control(bool);
        ~exception_control();
    };

}

#define ACTIVATE_EXCEPTIONS \
    ::test::exception_control BOOST_PP_CAT(ACTIVATE_EXCEPTIONS_, __LINE__) \
        (true)
#define DEACTIVATE_EXCEPTIONS \
    ::test::exception_control BOOST_PP_CAT(ACTIVATE_EXCEPTIONS_, __LINE__) \
        (false)

#endif

