
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_MEMBER_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_MEMBER_HEADER

#include <iosfwd>
#include <cstddef>

namespace test
{
    struct member
    {
        int value;

        explicit member(int x = 0);
        member(member const&);
        member& operator=(member const&);
        ~member();

        bool operator==(member const&) const;
        bool operator<(member const&) const;
    };

    std::ostream& operator<<(std::ostream&, member const&);
    test::member generate(test::member const*);
}

#if !defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
namespace test
#else
namespace boost
#endif
{
    std::size_t hash_value(test::member const& x);
}

#endif
