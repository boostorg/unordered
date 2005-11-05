
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./config.hpp"
#include "./member.hpp"
#include "./exception_trigger.hpp"
#include "./generators.hpp"

#if !defined(BOOST_OLD_IOSTREAMS)                
# include <ostream>
#else
# include <ostream.h>
#endif

namespace test
{
    member::member(int x)
        : value(x)
    {
        exception_trigger();
    }

    member::member(member const& x)
        : value(x.value)
    {
        exception_trigger();
    }

    member& member::operator=(member const& x)
    {
        exception_trigger();
        value = x.value;
        exception_trigger();

        return *this;
    }

    member::~member()
    {
        value = -1;
    }

    bool member::operator==(member const& x) const
    {
        exception_trigger();
        return value == x.value;
    }

    bool member::operator<(member const& x) const
    {
        exception_trigger();
        return value < x.value;
    }

    std::ostream& operator<<(std::ostream& out, member const& x)
    {
        out<<"Test class("<<x.value<<")";
        return out;
    }
    
    test::member generate(test::member const*)
    {
        return test::member(test::generator<int>()());
    }
}

#if !defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
namespace test
#else
namespace boost
#endif
{
    std::size_t hash_value(test::member const& x)
    {
        return x.value;
    }
}
