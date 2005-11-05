
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./exception.hpp"

namespace test
{
    exception::exception()
        : message_("Triggered test exception")
    {
    }

    exception::exception(char const* message)
        : message_(message)
    {
    }

    char const* exception::what() const throw()
    {
        return message_;
    }

    hash_exception::hash_exception()
        : exception("Triggered hash exception")
    {
    }

    hash_exception::hash_exception(char const* message)
        : exception(message)
    {
    }

    hash_copy_exception::hash_copy_exception()
        : hash_exception("Triggered hash copy exception")
    {
    }

    hash_copy_exception::hash_copy_exception(char const* message)
        : hash_exception(message)
    {
    }

    pred_exception::pred_exception()
        : exception("Triggered pred exception")
    {
    }

    pred_exception::pred_exception(char const* message)
        : exception(message)
    {
    }

    pred_copy_exception::pred_copy_exception()
        : pred_exception("Triggered pred copy exception")
    {
    }

    pred_copy_exception::pred_copy_exception(char const* message)
        : pred_exception(message)
    {
    }

    allocator_exception::allocator_exception()
        : exception("Triggered pred exception")
    {
    }

    allocator_exception::allocator_exception(char const* message)
        : exception(message)
    {
    }

    allocator_copy_exception::allocator_copy_exception()
        : allocator_exception("Triggered pred copy exception")
    {
    }

    allocator_copy_exception::allocator_copy_exception(char const* message)
        : allocator_exception(message)
    {
    }
}

