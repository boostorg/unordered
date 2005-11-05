
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_EQUIVALENT_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_EQUIVALENT_HEADER

#include <functional>
#include <boost/functional/hash.hpp>

namespace test
{
    template <class T>
    bool equivalent_impl(std::equal_to<T>, std::equal_to<T>, int)
    {
        return true;
    }

    template <class T>
    bool equivalent_impl(boost::hash<T>, boost::hash<T>, int)
    {
        return true;
    }

    template <class T>
    bool equivalent_impl(T const& x, T const& y, float)
    {
        return x == y;
    }

    template <class T>
    bool equivalent(T const& x, T const& y)
    {
        return equivalent_impl(x, y, 0);
    }

    template <class X1, class X2, class Y1, class Y2>
    bool equivalent(std::pair<X1, X2> const& x, std::pair<Y1, Y2> const& y)
    {
        return equivalent(x.first, y.first) && equivalent(x.second, y.second);
    }
}

#endif
