
//  Copyright Daniel James 2006. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_OBJECTS_MINIMAL_HEADER)
#define BOOST_UNORDERED_OBJECTS_MINIMAL_HEADER

#include <cstddef>

namespace test
{
namespace minimal
{
    class copy_constructible;
    class assignable;
    template <class T> class hash;
    template <class T> class equal_to;
    template <class T> class pointer;
    template <class T> class const_pointer;
    template <class T> class allocator;

    class copy_constructible
    {
    public:
        static copy_constructible create() { return copy_constructible(); }
        copy_constructible(copy_constructible const&) {}
        ~copy_constructible() {}
    private:
        copy_constructible& operator=(copy_constructible const&);
        copy_constructible() {}
    };

    class assignable
    {
    public:
        static assignable create() { return assignable(); }
        assignable(assignable const&) {}
        assignable& operator=(assignable const&) { return *this; }
        ~assignable() {}
    private:
        assignable() {}
    };

    template <class T>
    class hash
    {
    public:
        static hash create() { return hash(); }
        // TODO: hash has to be default constructible for the default
        // parameters. Maybe use an alternative version for testing
        // other member functions.
        //
        // Or maybe it's required to be default constructible?
        // The Container requirements include a default constructor.
        hash() {}
        hash(hash const&) {}
        // TODO: Required to be assignable?
        hash& operator=(hash const&) { return *this; }
        ~hash() {}

        std::size_t operator()(T const& x) const { return 0; }
    };

    template <class T>
    class equal_to
    {
    public:
        static equal_to create() { return equal_to(); }
        // TODO: equal_to has to be default constructible for the default
        // parameters. Maybe use an alternative version for testing
        // other member functions.
        //
        // Or maybe it's required to be default constructible?
        // The Container requirements include a default constructor.
        equal_to() {}
        equal_to(equal_to const&) {}
        // TODO: Required to be assignable?
        equal_to& operator=(equal_to const&) { return *this; }
        ~equal_to() {}

        bool operator()(T const& x, T const& y) const { return true; }
    };

    template <class T> class pointer;
    template <class T> class const_pointer;

    template <class T>
    class pointer
    {
        friend class allocator<T>;
        friend class const_pointer<T>;

        T* ptr_;

        pointer(T* ptr) : ptr_(ptr) {}
    public:
        pointer() : ptr_(0) {}

        typedef void (pointer::*bool_type)() const;
        void this_type_does_not_support_comparisons() const {}

        T& operator*() const { return *ptr_; }
        T* operator->() const { return ptr_; }
        pointer& operator++() { ++ptr_; return *this; }
        pointer operator++(int) { pointer tmp(*this); ++ptr_; return tmp; }
        pointer operator+(int s) const { return pointer(ptr_ + s); }
        T& operator[](int s) const { return ptr_[s]; }
        bool operator!() const { return !ptr_; }

        operator bool_type() const {
            return ptr_ ?
                &pointer::this_type_does_not_support_comparisons
                : 0;
        }

        bool operator==(pointer const& x) const { return ptr_ == x.ptr_; }
        bool operator!=(pointer const& x) const { return ptr_ != x.ptr_; }
        bool operator<(pointer const& x) const { return ptr_ < x.ptr_; }
        bool operator>(pointer const& x) const { return ptr_ > x.ptr_; }
        bool operator<=(pointer const& x) const { return ptr_ <= x.ptr_; }
        bool operator>=(pointer const& x) const { return ptr_ >= x.ptr_; }

        bool operator==(const_pointer<T> const& x) const { return ptr_ == x.ptr_; }
        bool operator!=(const_pointer<T> const& x) const { return ptr_ != x.ptr_; }
        bool operator<(const_pointer<T> const& x) const { return ptr_ < x.ptr_; }
        bool operator>(const_pointer<T> const& x) const { return ptr_ > x.ptr_; }
        bool operator<=(const_pointer<T> const& x) const { return ptr_ <= x.ptr_; }
        bool operator>=(const_pointer<T> const& x) const { return ptr_ >= x.ptr_; }
    };

    template <class T>
    class const_pointer
    {
        friend class allocator<T>;

        T* ptr_;

        const_pointer(T* ptr) : ptr_(ptr) {}
    public:
        const_pointer() : ptr_(0) {}
        const_pointer(pointer<T> const& x) : ptr_(x.ptr_) {}

        typedef void (const_pointer::*bool_type)() const;
        void this_type_does_not_support_comparisons() const {}

        T& operator*() const { return *ptr_; }
        T* operator->() const { return ptr_; }
        const_pointer& operator++() { ++ptr_; return *this; }
        const_pointer operator++(int) { const_pointer tmp(*this); ++ptr_; return tmp; }
        const_pointer operator+(int s) const { return const_pointer(ptr_ + s); }
        T& operator[](int s) const { return ptr_[s]; }
        bool operator!() const { return !ptr_; }

        operator bool_type() const {
            return ptr_ ?
                &const_pointer::this_type_does_not_support_comparisons
                : 0;
        }

        bool operator==(pointer<T> const& x) const { return ptr_ == x.ptr_; }
        bool operator!=(pointer<T> const& x) const { return ptr_ != x.ptr_; }
        bool operator<(pointer<T> const& x) const { return ptr_ < x.ptr_; }
        bool operator>(pointer<T> const& x) const { return ptr_ > x.ptr_; }
        bool operator<=(pointer<T> const& x) const { return ptr_ <= x.ptr_; }
        bool operator>=(pointer<T> const& x) const { return ptr_ >= x.ptr_; }

        bool operator==(const_pointer const& x) const { return ptr_ == x.ptr_; }
        bool operator!=(const_pointer const& x) const { return ptr_ != x.ptr_; }
        bool operator<(const_pointer const& x) const { return ptr_ < x.ptr_; }
        bool operator>(const_pointer const& x) const { return ptr_ > x.ptr_; }
        bool operator<=(const_pointer const& x) const { return ptr_ <= x.ptr_; }
        bool operator>=(const_pointer const& x) const { return ptr_ >= x.ptr_; }
    };

    template <class T>
    class allocator
    {
    public:
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;
        typedef pointer<T> pointer;
        typedef const_pointer<T> const_pointer;
        typedef T& reference;
        typedef T const& const_reference;
        typedef T value_type;

        template <class U> struct rebind { typedef allocator<U> other; };

        allocator() {}
        template <class Y> allocator(allocator<Y> const&) {}
        allocator(allocator const&) {}
        ~allocator() {}

        pointer address(reference r) { return pointer(&r); }
        const_pointer address(const_reference r) { return const_pointer(&r); }

        pointer allocate(size_type n) {
            return pointer(static_cast<T*>(::operator new(n * sizeof(T))));
        }

        pointer allocate(size_type n, const_pointer u)
        {
            return pointer(static_cast<T*>(::operator new(n * sizeof(T))));
        }

        void deallocate(pointer p, size_type n)
        {
            ::operator delete((void*) p.ptr_);
        }

        void construct(pointer p, T const& t) { new((void*)p.ptr_) T(t); }
        void destroy(pointer p) { ((T*)p.ptr_)->~T(); }

        size_type max_size() const { return 1000; }

#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
    public: allocator& operator=(allocator const&) { return *this;}
#else
    private: allocator& operator=(allocator const&);
#endif
    };

    template <class T>
    inline bool operator==(allocator<T> const& x, allocator<T> const& y)
    {
        return true;
    }

    template <class T>
    inline bool operator!=(allocator<T> const& x, allocator<T> const& y)
    {
        return false;
    }

    template <class T>
    void swap(allocator<T>& x, allocator<T>& y)
    {
    }
}
}

#endif
