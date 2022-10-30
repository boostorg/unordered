// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/unordered/hash_traits.hpp>
#include <boost/core/lightweight_test_trait.hpp>

struct X1
{
};

struct X2
{
    typedef void is_avalanching;
};

int main()
{
    using boost::unordered::hash_is_avalanching;

    BOOST_TEST_TRAIT_FALSE((hash_is_avalanching<X1>));
    BOOST_TEST_TRAIT_TRUE((hash_is_avalanching<X2>));

    return boost::report_errors();
}
