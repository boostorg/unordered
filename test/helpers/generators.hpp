
// Copyright 2005-2006 Daniel James.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// A crude wrapper round Boost.Random to make life easier.

#if !defined(BOOST_UNORDERED_TEST_HELPERS_GENERATORS_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_GENERATORS_HEADER

#include <string>
#include <utility>
#include <stdexcept>
#include <boost/random/inversive_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/lagged_fibonacci.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include "./fwd.hpp"

namespace test
{
    typedef boost::hellekalek1995 integer_generator_type;
    typedef boost::lagged_fibonacci607 real_generator_type;

    template <class T>
    struct generator;

    template <class T1, class T2> std::pair<T1, T2> generate(
            std::pair<T1, T2> const*)
    {
        static generator<T1> g1;
        static generator<T2> g2;

        return std::pair<T1, T2>(g1(), g2());
    }

    template <class T>
    struct generator
    {
        typedef T value_type;
        value_type operator()()
        {
            return generate((T const*) 0);
        }
    };

    inline int generate(int const*)
    {
        static integer_generator_type gen;
        static boost::uniform_int<> dist(0, 1000);
        static boost::variate_generator<integer_generator_type, boost::uniform_int<> >
            vg(gen, dist);
        return vg();
    }

    inline char generate(char const*)
    {
        static integer_generator_type gen;
        static boost::uniform_int<char> dist(32, 127);
        static boost::variate_generator<integer_generator_type, boost::uniform_int<char> >
            vg(gen, dist);
        return vg();
    }

    inline std::string generate(std::string const*)
    {
        using namespace std;

        static test::generator<char> char_gen;

        std::string result;

        int length = rand() % 10;
        for(int i = 0; i < length; ++i)
            result += char_gen();

        return result;
    }

    float generate(float const*)
    {
        static real_generator_type gen;
        static boost::uniform_real<float> dist;
        static boost::variate_generator<real_generator_type, boost::uniform_real<float> >
            vg(gen, dist);
        return vg();
    }
}

#endif
