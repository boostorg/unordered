
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_GENERATORS_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_GENERATORS_HEADER

#include <cstdlib>
#include <string>
#include <utility>

namespace test
{
    int generate(int const*);
    char generate(char const*);
    std::string generate(std::string const*);
    template <class T1, class T2> std::pair<T1, T2> generate(
            std::pair<T1, T2>  const*);

    template <class T>
    struct generator
    {
        typedef T value_type;
        value_type operator()()
        {
            return generate((T  const*) 0);
        }
    };

    template <class T1, class T2>
    std::pair<T1, T2> generate(std::pair<T1, T2>  const*)
    {
        static generator<T1> g1;
        static generator<T2> g2;

        return std::pair<T1, T2>(g1(), g2());
    }
}

#endif
