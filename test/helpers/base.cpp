
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./base.hpp"
#include <vector>
#include <algorithm>

namespace test
{
    namespace
    {
        std::vector<void(*)()> end_checks;
    }

    void register_end_check(void(*check)())
    {
        end_checks.push_back(check);
    }

    void call_check(void(*check)())
    {
        check();
    }

    void end()
    {
        std::for_each(end_checks.begin(), end_checks.end(), call_check);
    }
}
