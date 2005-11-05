
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./config.hpp"
#include "./allocator.hpp"
#include "./exception_trigger.hpp"
#include "./exception.hpp"
#include <boost/test/test_tools.hpp>
#include <map>

#if !defined(BOOST_OLD_IOSTREAMS)                
# include <ostream>
#else
# include <ostream.h>
#endif

namespace test
{
    namespace
    {
        const unsigned int max_track = 1000;

        struct allocate_details
        {
            int tag;
            std::size_t length;

            allocate_details()
                : tag(0), length(0) {}

            allocate_details(int t, int l)
                : tag(t), length(l) {}
        };

        std::map<void*, allocate_details> allocate_map;
        unsigned int reference_count = 0;
        unsigned int alloc_count = 0;

        static void ref()
        {
            ++reference_count;
        }

        static void unref()
        {
            if(--reference_count == 0) {
                BOOST_CHECK_MESSAGE(alloc_count == 0 && allocate_map.empty(),
                        "Memory leak found");
                allocate_map.clear();
            }
        }
    }

    allocator_base::allocator_base(int x)
        : tag(x)
    {
        ref();
    }

    allocator_base::allocator_base(allocator_base const& x)
        : tag(x.tag)
    {
        ref();
    }

    allocator_base::~allocator_base()
    {
        unref();
    }

    allocator_base::size_type allocator_base::max_size() const
    {
        return (std::numeric_limits<size_type>::max)();
    }

    void* allocator_base::allocate(size_type n, void const*, size_type size)
    {
        BOOST_CHECK(n <= max_size());

        exception_trigger((allocator_exception*) 0);

        // TODO: This is not exception safe.
        void* ptr = ::operator new(n * size);
        ++alloc_count;
        if(allocate_map.size() < max_track)
            allocate_map[ptr] = allocate_details(tag, n);

        return ptr;
    }

    void allocator_base::construct(void* ptr)
    {
        exception_trigger((allocator_exception*) 0);
    }

    void allocator_base::destroy(void* ptr)
    {
    }

    void allocator_base::deallocate(void* ptr, size_type n)
    {
        BOOST_CHECK(n <= max_size());
        if(allocate_map.find(ptr) == allocate_map.end()) {
            if(alloc_count <= allocate_map.size())
                BOOST_ERROR("Deallocating unknown pointer.");
        } else {
            // TODO: This is not exception safe.
            BOOST_CHECK_EQUAL(allocate_map[ptr].tag, tag);
            BOOST_CHECK_EQUAL(allocate_map[ptr].length, n);
            allocate_map.erase(ptr);
            ::operator delete(ptr);
        }
        --alloc_count;
    }

    void allocator_base::swap(allocator_base& x)
    {
        std::swap(tag, x. tag);
    }

    std::ostream& operator<<(std::ostream& out, allocator_base const& x)
    {
        out<<"Test Allocator("<<x.tag<<")\n";
        return out;
    }

    bool allocator_equals(allocator_base const& x, allocator_base const& y)
    {
        return x.tag == y.tag;
    }

//////////////////////////////////////////////////////////////////////////////

    minimal_allocator_base::minimal_allocator_base(int x)
        : tag(x)
    {
        ref();
    }

    minimal_allocator_base::minimal_allocator_base(minimal_allocator_base const& x)
        : tag(x.tag)
    {
        ref();
    }

    minimal_allocator_base::~minimal_allocator_base()
    {
        unref();
    }

    minimal_allocator_base::size_type minimal_allocator_base::max_size() const
    {
        return (std::numeric_limits<size_type>::max)() / 4;
    }

    void* minimal_allocator_base::allocate(size_type n, void const*, size_type size)
    {
        BOOST_CHECK(n <= max_size());

        exception_trigger((allocator_exception*) 0);

        // TODO: This is not exception safe.
        void* ptr = ::operator new(n * size);
        allocate_map[ptr] = allocate_details(tag, n);

        return ptr;
    }

    void minimal_allocator_base::construct(void* ptr)
    {
        exception_trigger((allocator_exception*) 0);
    }

    void minimal_allocator_base::destroy(void* ptr)
    {
    }

    void minimal_allocator_base::deallocate(void* ptr, size_type n)
    {
        BOOST_CHECK(n <= max_size());
        if(allocate_map.find(ptr) == allocate_map.end()) {
            BOOST_ERROR("Deallocating unknown pointer.");
        } else {
            // TODO: This is not exception safe.
            BOOST_CHECK_EQUAL(allocate_map[ptr].tag, tag);
            BOOST_CHECK_EQUAL(allocate_map[ptr].length, n);
            allocate_map.erase(ptr);
            ::operator delete(ptr);
        }
    }

    void minimal_allocator_base::swap(minimal_allocator_base& x)
    {
        std::swap(tag, x. tag);
    }

    std::ostream& operator<<(std::ostream& out, minimal_allocator_base const& x)
    {
        out<<"Minimal Allocator("<<x.tag<<")\n";
        return out;
    }

    bool allocator_equals(minimal_allocator_base const& x, minimal_allocator_base const& y)
    {
        return x.tag == y.tag;
    }
}
