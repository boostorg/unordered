
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_EXCEPTION_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_EXCEPTION_HEADER

#include <exception>

namespace test
{
    // Exception Handling

    struct exception : std::exception
    {
        char const* message_;
        exception();
        exception(char const*);
        char const* what() const throw();
    };

    struct hash_exception : exception
    {
        hash_exception();
        hash_exception(char const*);
    };

    struct hash_copy_exception : hash_exception
    {
        hash_copy_exception();
        hash_copy_exception(char const*);
    };

    struct pred_exception : exception
    {
        pred_exception();
        pred_exception(char const*);
    };

    struct pred_copy_exception : pred_exception
    {
        pred_copy_exception();
        pred_copy_exception(char const*);
    };

    struct allocator_exception : exception
    {
        allocator_exception();
        allocator_exception(char const*);
    };

    struct allocator_copy_exception : allocator_exception
    {
        allocator_copy_exception();
        allocator_copy_exception(char const*);
    };
}

#endif
