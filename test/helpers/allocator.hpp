
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_ALLOCATOR_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_ALLOCATOR_HEADER

#include <iosfwd>
#include <cstddef>
#include <boost/limits.hpp>

namespace test
{
    struct allocator_base
    {
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        int tag;

        allocator_base(int x);
        allocator_base(allocator_base const&);
        ~allocator_base();

        size_type max_size() const;

        void* allocate(size_type, void const*, size_type);
        void construct(void*);
        void destroy(void*);
        void deallocate(void*, size_type);
        void swap(allocator_base&);
    private:
        allocator_base& operator=(allocator_base const&);
    };

    std::ostream& operator<<(std::ostream&, allocator_base const&);
    bool allocator_equals(allocator_base const& x, allocator_base const& y);

    template <class T>
    struct allocator : allocator_base
    {
        typedef T* pointer;
        typedef T const* const_pointer;
        typedef T& reference;
        typedef T const& const_reference;
        typedef T value_type;

        template <class T1>
        struct rebind
        {
            typedef allocator<T1> other;
        };

        pointer address(reference x) const
        {
            return &x;
        }

        const_pointer address(const_reference x) const
        {
            return &x;
        }

        allocator(int x = 1) : allocator_base(x) {}
        template <class T1>
        allocator(allocator<T1> const& x) : allocator_base(x) {}
        allocator(allocator const& x) : allocator_base(x) {}
        ~allocator() {}

        pointer allocate(size_type n, T const* hint = 0)
        {
            return static_cast<pointer>(
                    allocator_base::allocate(n, hint, sizeof(T)));
        }

        void construct(pointer ptr, T const& x)
        {
            allocator_base::construct(ptr);
            new((void*)ptr)T(x);
        }

        void destroy(pointer ptr)
        {
            allocator_base::destroy(ptr);
            ptr->~T();
        }

        void deallocate(pointer ptr, size_type n)
        {
            allocator_base::deallocate(ptr, n);
        }
    };

    template <class T>
    inline bool operator==(allocator<T> const& x, allocator<T> const& y)
    {
        return test::allocator_equals(x, y);
    }

    template <class T>
    inline bool operator!=(allocator<T> const& x, allocator<T> const& y)
    {
        return !test::allocator_equals(x, y);
    }

    template <class T>
    void swap(allocator<T>& x, allocator<T>& y)
    {
        x.swap(y);
    }

    template <class T>
    allocator<T> create_allocator(allocator<T>*)
    {
        return allocator<T>();
    }

    template <class T>
    allocator<T> create_allocator(allocator<T>*, int x)
    {
        return allocator<T>(x);
    }

    template <class T> struct minimal_allocator;
    typedef unsigned short minimal_size_type;

    template <class Ptr, class T>
    class minimal_pointer_base
    {
    protected:
        typedef minimal_pointer_base<Ptr, T> pointer_base;
        minimal_pointer_base() : ptr_(0) {}
        explicit minimal_pointer_base(T* ptr) : ptr_(ptr) {}
        ~minimal_pointer_base() {}
        Ptr& get() { return *static_cast<Ptr*>(this); }
        T* ptr_;
    public:
        typedef void (minimal_pointer_base::*bool_type)() const;
        void this_type_does_not_support_comparisons() const {}

        T& operator*() const { return *ptr_; }
        T* operator->() const { return ptr_; }
        Ptr& operator++() { ++ptr_; return get(); }
        Ptr operator++(int) { Ptr tmp(get()); ++ptr_; return tmp; }

        Ptr operator+(minimal_size_type s) const
        {
            return Ptr(ptr_ + s);
        }

        T& operator[](minimal_size_type s) const
        {
            return ptr_[s];
        }

        operator bool_type() const
        {
            return ptr_ ?
                &minimal_pointer_base::this_type_does_not_support_comparisons
                : 0;
        }

        bool operator!() const { return !ptr_; }
        bool operator==(Ptr const& x) const { return ptr_ == x.ptr_; }
        bool operator!=(Ptr const& x) const { return ptr_ != x.ptr_; }
        bool operator<(Ptr const& x) const { return ptr_ < x.ptr_; }
        bool operator>(Ptr const& x) const { return ptr_ > x.ptr_; }
        bool operator<=(Ptr const& x) const { return ptr_ <= x.ptr_; }
        bool operator>=(Ptr const& x) const { return ptr_ >= x.ptr_; }

        friend std::ostream& operator<<(std::ostream& out, minimal_pointer_base const& x)
        {
            out<<x.ptr_;
            return out;
        }
    };

    template <class T> class minimal_pointer;
    template <class T> class minimal_const_pointer;

    template <class T>
    class minimal_pointer
        : public minimal_pointer_base<minimal_pointer<T>, T>
    {
        friend struct minimal_allocator<T>;
        friend class minimal_pointer_base<minimal_pointer<T>, T>;
        friend class minimal_const_pointer<T>;
        typedef typename minimal_pointer::pointer_base base;
        minimal_pointer(T* ptr) : base(ptr) {}

        typedef minimal_const_pointer<T> const_pointer;
        typedef minimal_pointer<T> pointer;
    public:
        minimal_pointer() : base() {}

        bool operator==(pointer const& x) const { return base::operator==(x); }
        bool operator!=(pointer const& x) const { return base::operator!=(x); }
        bool operator<(pointer const& x) const { return  base::operator<(x);}
        bool operator>(pointer const& x) const { return  base::operator>(x);}
        bool operator<=(pointer const& x) const { return  base::operator<=(x);}
        bool operator>=(pointer const& x) const { return  base::operator<=(x);}

        bool operator==(const_pointer const& x) const { return x == *this; }
        bool operator!=(const_pointer const& x) const { return x != *this; }
        bool operator<(const_pointer const& x) const { return x > *this; }
        bool operator>(const_pointer const& x) const { return x < *this; }
        bool operator<=(const_pointer const& x) const { return x >= *this; }
        bool operator>=(const_pointer const& x) const { return x <= *this; }
    };

    template <class T>
    class minimal_const_pointer
        : public minimal_pointer_base<minimal_const_pointer<T>, T const>
    {
        friend struct minimal_allocator<T>;
        friend class minimal_pointer_base<minimal_const_pointer<T>, T const>;
        typedef typename minimal_const_pointer::pointer_base base;
        minimal_const_pointer(T* ptr) : base(ptr) {}

        typedef minimal_const_pointer<T> const_pointer;
        typedef minimal_pointer<T> pointer;
    public:
        minimal_const_pointer() : base() {}
        minimal_const_pointer(minimal_pointer<T> const& x) : base(x.ptr_) {}

        bool operator==(const_pointer const& x) const { return base::operator==(x); }
        bool operator!=(const_pointer const& x) const { return base::operator!=(x); }
        bool operator<(const_pointer const& x) const { return base::operator<(x);}
        bool operator>(const_pointer const& x) const { return base::operator>(x);}
        bool operator<=(const_pointer const& x) const { return base::operator<=(x);}
        bool operator>=(const_pointer const& x) const { return base::operator<=(x);}

        bool operator==(pointer const& x) const { return operator==(const_pointer(x)); }
        bool operator!=(pointer const& x) const { return operator!=(const_pointer(x)); }
        bool operator<(pointer const& x) const { return operator<(const_pointer(x));}
        bool operator>(pointer const& x) const { return operator>(const_pointer(x));}
        bool operator<=(pointer const& x) const { return operator<=(const_pointer(x));}
        bool operator>=(pointer const& x) const { return operator<=(const_pointer(x));}
    };

    struct minimal_allocator_base
    {
        typedef minimal_size_type size_type;
        typedef std::ptrdiff_t difference_type;

        int tag;

        minimal_allocator_base(int x);
        minimal_allocator_base(minimal_allocator_base const&);
        ~minimal_allocator_base();

        size_type max_size() const;

        void* allocate(size_type, void const*, size_type);
        void construct(void*);
        void destroy(void*);
        void deallocate(void*, size_type);
        void swap(minimal_allocator_base&);
    private:
        minimal_allocator_base& operator=(minimal_allocator_base const&);
    };

    std::ostream& operator<<(std::ostream&, minimal_allocator_base const&);
    bool allocator_equals(minimal_allocator_base const&, minimal_allocator_base const&);

    template <class T>
    struct minimal_allocator : minimal_allocator_base
    {
        typedef minimal_pointer<T> pointer;
        typedef minimal_const_pointer<T> const_pointer;
        typedef T& reference;
        typedef T const& const_reference;
        typedef T value_type;

        template <class U>
        struct rebind
        {
            typedef minimal_allocator<U> other;
        };

        pointer address(reference r)
        {
            return pointer(&r);
        }

        const_pointer address(const_reference r)
        {
            return const_pointer(&r);
        }

        pointer allocate(size_type n)
        {
            return pointer(static_cast<T*>(
                    minimal_allocator_base::allocate(n, 0, sizeof(T))));
        }

        pointer allocate(size_type n, const_pointer u)
        {
            return pointer(static_cast<T*>(
                    minimal_allocator_base::allocate(n, u.ptr_, sizeof(T))));
        }

        void deallocate(pointer p, size_type n)
        {
            minimal_allocator_base::deallocate(p.ptr_, n);
        }

        minimal_allocator()
            : minimal_allocator_base(0)
        {
        }

        explicit minimal_allocator(int tag)
            : minimal_allocator_base(tag)
        {
        }

        template <class Y>
        minimal_allocator(minimal_allocator<Y> const& x)
            : minimal_allocator_base(x)
        {
        }

        minimal_allocator(minimal_allocator const& x)
            : minimal_allocator_base(x)
        {
        }

        void construct(pointer p, T const& t)
        {
            minimal_allocator_base::construct(p.ptr_);
            new((void*)p.ptr_) T(t);
        }

        void destroy(pointer p)
        {
            minimal_allocator_base::destroy(p.ptr_);
            ((T*)p.ptr_)->~T();
        }
    private:
        minimal_allocator& operator=(minimal_allocator const&);
    };

    template <class T>
    inline bool operator==(minimal_allocator<T> const& x, minimal_allocator<T> const& y)
    {
        return test::allocator_equals(x, y);
    }

    template <class T>
    inline bool operator!=(minimal_allocator<T> const& x, minimal_allocator<T> const& y)
    {
        return !test::allocator_equals(x, y);
    }

    template <class T>
    void swap(minimal_allocator<T>& x, minimal_allocator<T>& y)
    {
        x.swap(y);
    }

    template <class T>
    minimal_allocator<T> create_allocator(minimal_allocator<T>*)
    {
        return minimal_allocator<T>();
    }

    template <class T>
    minimal_allocator<T> create_allocator(minimal_allocator<T>*, int x)
    {
        return minimal_allocator<T>(x);
    }
}

#endif
