
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_INVARIANT_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_INVARIANT_HEADER

#include <boost/preprocessor/cat.hpp>
#include "./exception_trigger.hpp"

namespace test
{
    struct invariant_checker_base
    {
        invariant_checker_base();
        virtual void check() const = 0;
    protected:
        virtual ~invariant_checker_base();
    };

    void check_invariants();
    void invariant_add(invariant_checker_base*);
    void invariant_remove(invariant_checker_base*);

    template <class T>
    struct invariant_checker : invariant_checker_base
    {
        T& object_;

        invariant_checker(T& o) : object_(o)
        {
            invariant_add(this);
        }

        ~invariant_checker()
        {
            check();
            invariant_remove(this);
        }

        void check() const
        {
            DEACTIVATE_EXCEPTIONS;
            invariant_impl(object_);
        }
    };

    // On compilers without RVO check() will be called multiple times.
    // No big deal I suppose.
    template <class T>
    invariant_checker<T> make_invariant_checker(T& o)
    {
        return invariant_checker<T>(o);
    }

    // Calling this also prevents an unused variable warning when using
    // an invariant checker.
    void initial_check(::test::invariant_checker_base const&);

    // A one time invariant check.
    template <class T>
    void invariant_check(T const& x)
    {
        DEACTIVATE_EXCEPTIONS;
        invariant_impl(x);
    }
}

#define INVARIANT_CHECK(o) \
    INVARIANT_CHECK2(o, BOOST_PP_CAT(invariant_checker_, __LINE__))

#define INVARIANT_CHECK2(o, name) \
    ::test::invariant_checker_base const& name = \
        ::test::make_invariant_checker(o); \
    ::test::initial_check(name)

#endif
