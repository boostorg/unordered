
//  Copyright Daniel James 2005-2006. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_RANDOM_VALUES_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_RANDOM_VALUES_HEADER

#include <vector>
#include <algorithm>
#include "./generators.hpp"
#include "./metafunctions.hpp"

namespace test
{
    template <class X>
    struct random_values
        : public std::vector<typename non_const_value_type<X>::type>
    {
        random_values(int count) {
            typedef typename non_const_value_type<X>::type value_type;
            static test::generator<value_type> gen;
            this->reserve(count);
            std::generate_n(std::back_inserter(*this), count, gen);
        }
    };
}

#endif
