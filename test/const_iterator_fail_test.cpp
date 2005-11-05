
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/unordered_map.hpp>

void func()
{
    typedef boost::unordered_map<int, int> map;
    typedef map::iterator iterator;
    typedef map::const_iterator const_iterator;

    const_iterator x;
    iterator y(x);
}
