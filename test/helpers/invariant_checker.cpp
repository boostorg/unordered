
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/config.hpp>
#include "./invariant_checker.hpp"
#include <set>
#include <algorithm>
#include <boost/functional.hpp>

namespace test
{
    namespace
    {
        typedef std::set<invariant_checker_base*> check_set;
        check_set checks;
    }

    invariant_checker_base::invariant_checker_base()
    {
    }

    invariant_checker_base::~invariant_checker_base()
    {
    }

    void check_invariants()
    {
        // This was causing compile errors on Visual C++, because check
        // has return type void.
        //std::for_each(checks.begin(), checks.end(),
        //        boost::mem_fun(&invariant_checker_base::check));

        check_set::iterator end = checks.end();
        for(check_set::iterator it = checks.begin(); it != end; ++it) 
            (*it)->check();
    }

    void invariant_add(invariant_checker_base* check)
    {
        checks.insert(check);
    }

    void invariant_remove(invariant_checker_base* check)
    {
        checks.erase(check);
    }

    void initial_check(invariant_checker_base const& x)
    {
        x.check();
    }
}
