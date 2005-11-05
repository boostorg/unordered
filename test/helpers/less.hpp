
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_LESS_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_LESS_HEADER

#include <utility>

namespace test
{
    template <class T>
    bool compare_impl(T const& x, T const& y, float)
    {
        return x < y;
    }

    template <class T1, class T2>
    bool compare_impl(std::pair<T1, T2> const& x,
            std::pair<T1, T2> const& y, int)
    {
        return x.first < y.first ||
            (x.first == y.first && x.second < y.second);
    }

    struct compare {
        template <class T>
        bool operator()(T const& x, T const& y)
        {
            return compare_impl(x, y, 0);
        }
    };
}

#endif

