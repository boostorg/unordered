
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_FUNCTIONAL_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_FUNCTIONAL_HEADER

#include <iosfwd>
#include <boost/functional/hash.hpp>

namespace test
{
    struct hash
    {
        int offset;

        explicit hash(int x = 1);
        hash(hash const&);
        hash& operator=(hash const&);

        std::size_t calculate_hash(std::size_t) const;

        template <class T>
        std::size_t operator()(T const& x) const
        {
            return calculate_hash(boost::hash<T>()(x));
        }

        std::size_t operator()(char const* x) const;
        bool operator==(hash const& x) const;
    };

    std::ostream& operator<<(std::ostream& out, hash x);
    hash create_hasher(hash*);
    hash create_hasher(hash*, int x);

    struct equals
    {
        int tag;

        explicit equals(int x = 1);
        equals(equals const&);
        equals& operator=(equals const&);

        bool calculate_equals(bool) const;

        template <class T1, class T2>
        bool operator()(T1 const& x, T2 const& y) const
        {
            return calculate_equals(x == y);
        }

        bool operator()(char const*, char const*) const;
        bool operator==(equals const& x) const;
    };

    std::ostream& operator<<(std::ostream& out, equals x);
    equals create_key_equal(equals*);
    equals create_key_equal(equals*, int x);

    struct less
    {
        int tag;

        explicit less(int x = 0);

        template <class T1, class T2>
        bool operator()(T1 const& x, T2 const& y) const
        {
            return x < y;
        }

        bool operator()(char const*, char const*) const;
    };

    std::ostream& operator<<(std::ostream& out, less x);
}

#endif
