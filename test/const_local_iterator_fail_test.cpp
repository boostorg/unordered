
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/unordered_map.hpp>

void func()
{
    // This is only required to fail for unordered maps & multimaps as for sets
    // and multisets both iterator and const_iterator are const.
    typedef boost::unordered_map<int, int> map;
    typedef map::local_iterator local_iterator;
    typedef map::const_local_iterator const_local_iterator;

    const_local_iterator x;
    local_iterator y(x);
}
