
//  Copyright Daniel James 2006. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_HEADER

namespace test
{
    template <class Container>
    inline typename Container::key_type get_key(typename Container::key_type const& x)
    {
        return x;
    }

    template <class Container, class T>
    inline typename Container::key_type get_key(std::pair<typename Container::key_type const, T> const& x)
    {
        return x.first;
    }

    template <class Container, class T>
    inline typename Container::key_type get_key(std::pair<typename Container::key_type, T> const& x)
    {
        return x.first;
    }
}

#endif
