
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "generators.hpp"

// Generators

namespace test
{
    int generate(int const*)
    {
        using namespace std;
        return rand();
    }

    char generate(char const*)
    {
        using namespace std;
        return (char)(rand() % 26) + 'a';
    }

    std::string generate(std::string const*)
    {
        using namespace std;

        static test::generator<char> char_gen;

        std::string result;

        int length = rand() % 10;
        for(int i = 0; i < length; ++i)
            result += char_gen();

        //std::generate_n(
        //        std::back_inserter(result),
        //        rand() % 10,
        //        char_gen);

        return result;
    }
}
