
//  Copyright Daniel James 2006. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_FWD_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_FWD_HEADER

namespace test
{
    int generate(int const*);
    char generate(char const*);
    std::string generate(std::string*);
    float generate(float const*);
    template <class T1, class T2>
    std::pair<T1, T2> generate(std::pair<T1, T2>*);
}

#endif

