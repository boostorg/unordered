
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/unordered_set.hpp>

void func()
{
    boost::unordered_set<int> x;
    x.insert(10);
    boost::unordered_set<int>::iterator it = x.begin();
    *x = 25;
}
