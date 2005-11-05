
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_CONSTRUCTORS_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_CONSTRUCTORS_HEADER

namespace test
{
    template <class Allocator>
    Allocator create_allocator(Allocator*, int = 0)
    {
        return Allocator();
    }

    template <class Hasher>
    Hasher create_hasher(Hasher*, int = 0)
    {
        return Hasher();
    }

    template <class KeyEqual>
    KeyEqual create_key_equal(KeyEqual*, int = 0)
    {
        return KeyEqual();
    }

    template <class Container>
    class constructors
    {
    public:
        typedef typename Container::allocator_type allocator_type;
        typedef typename Container::hasher hasher_type;
        typedef typename Container::key_equal key_equal_type;

        allocator_type allocator() const
        {
            return create_allocator((allocator_type*) 0);
        }

        allocator_type allocator(int x) const
        {
            return create_allocator((allocator_type*) 0, x);
        }

        hasher_type hasher() const
        {
            return create_hasher((hasher_type*) 0);
        }

        hasher_type hasher(int x) const
        {
            return create_hasher((hasher_type*) 0, x);
        }

        key_equal_type key_equal() const
        {
            return create_key_equal((key_equal_type*) 0);
        }

        key_equal_type key_equal(int x) const
        {
            return create_key_equal((key_equal_type*) 0, x);
        }
    };
}

#endif
