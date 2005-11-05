
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TESTS_CHECK_RETURN_TYPE_HEADER)
#define BOOST_UNORDERED_TESTS_CHECK_RETURN_TYPE_HEADER

#include <boost/mpl/assert.hpp>

template <class T1>
struct check_return_type
{
    template <class T2>
    static int equals(T2)
    {
        BOOST_MPL_ASSERT((boost::is_same<T1, T2>));
        return 0;
    }

    template <class T2>
    static int equals_ref(T2&)
    {
        BOOST_MPL_ASSERT((boost::is_same<T1, T2>));
        return 0;
    }

    template <class T2>
    static int convertible(T2)
    {
        BOOST_MPL_ASSERT((boost::is_convertible<T2, T1>));
        return 0;
    }
};

#endif
