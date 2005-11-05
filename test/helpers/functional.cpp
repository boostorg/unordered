
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./config.hpp"
#include "./functional.hpp"
#include "./exception.hpp"
#include "./exception_trigger.hpp"
#include <cstddef>
#include <string>

#if !defined(BOOST_OLD_IOSTREAMS)
# include <ostream>
#else
# include <ostream.h>
#endif

namespace test
{
    // Hash

    hash::hash(int x)
        : offset(x) {}

    hash::hash(hash const& x)
        : offset(x.offset)
    {
        exception_trigger((hash_copy_exception*) 0);
    }

    hash& hash::operator=(hash const& x)
    {
        exception_trigger((hash_copy_exception*) 0);
        offset = x.offset;
        exception_trigger((hash_copy_exception*) 0);

        return *this;
    }

    std::size_t hash::calculate_hash(std::size_t x) const
    {
        exception_trigger((hash_exception*) 0);
        return x + offset;
    }

    std::size_t hash::operator()(char const* x) const
    {
        return calculate_hash(boost::hash<std::string>()(x));
    }

    bool hash::operator==(hash const& x) const
    {
        return offset == x.offset;
    }

    std::ostream& operator<<(std::ostream& out, hash x)
    {
        out<<"Test Hash("<<x.offset<<")\n";
        return out;
    }

    hash create_hasher(hash*)
    {
        return hash();
    }

    hash create_hasher(hash*, int x)
    {
        return hash(x);
    }

    // Equals

    equals::equals(int x)
        : tag(x) {}

    equals::equals(equals const& x)
        : tag(x.tag)
    {
        exception_trigger((pred_copy_exception*) 0);
    }

    equals& equals::operator=(equals const& x)
    {
        exception_trigger((pred_copy_exception*) 0);
        tag = x.tag;
        exception_trigger((pred_copy_exception*) 0);

        return *this;
    }

    bool equals::calculate_equals(bool x) const
    {
        exception_trigger((pred_exception*) 0);
        return x;
    }

    bool equals::operator()(char const* x, char const* y) const
    {
        return calculate_equals(std::string(x) == std::string(y));
    }

    bool equals::operator==(equals const& x) const
    {
        return tag == x.tag;
    }

    std::ostream& operator<<(std::ostream& out, equals x)
    {
        out<<"Test Equals("<<x.tag<<")\n";
        return out;
    }

    equals create_key_equal(equals*)
    {
        return equals();
    }

    equals create_key_equal(equals*, int x)
    {
        return equals(x);
    }

    less::less(int x)
        : tag(x)
    {
    }

    bool less::operator()(char const* x, char const* y) const
    {
        return std::string(x) < std::string(y);
    }

    std::ostream& operator<<(std::ostream& out, less x)
    {
        out<<"Test Less("<<x.tag<<")\n";
        return out;
    }
}
