
// Copyright (C) 2003-2004 Jeremy B. Maitin-Shepard.
// Copyright (C) 2005-2011, 2014 Daniel James
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNORDERED_DETAIL_ALL_HPP_INCLUDED
#define BOOST_UNORDERED_DETAIL_ALL_HPP_INCLUDED

#include <boost/config.hpp>
#if defined(BOOST_HAS_PRAGMA_ONCE)
#pragma once
#endif

#include <boost/unordered/detail/allocate.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/type_traits/is_nothrow_move_constructible.hpp>
#include <boost/type_traits/is_nothrow_move_assignable.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_empty.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/detail/select_type.hpp>
#include <boost/move/move.hpp>
#include <boost/swap.hpp>
#include <boost/assert.hpp>
#include <boost/limits.hpp>
#include <boost/iterator.hpp>
#include <cmath>

#if defined(BOOST_MSVC)
#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant
#endif

#if defined(BOOST_UNORDERED_DEPRECATED_EQUALITY)

#if defined(__EDG__)
#elif defined(_MSC_VER) || defined(__BORLANDC__) || defined(__DMC__)
#pragma message("Warning: BOOST_UNORDERED_DEPRECATED_EQUALITY is no longer supported.")
#elif defined(__GNUC__) || defined(__HP_aCC) || \
    defined(__SUNPRO_CC) || defined(__IBMCPP__)
#warning "BOOST_UNORDERED_DEPRECATED_EQUALITY is no longer supported."
#endif

#endif

namespace boost { namespace unordered { namespace detail {

    static const float minimum_max_load_factor = 1e-3f;
    static const std::size_t default_bucket_count = 11;
    struct move_tag {};
    struct empty_emplace {};

    namespace func {
        template <class T>
        inline void ignore_unused_variable_warning(T const&) {}
    }

    ////////////////////////////////////////////////////////////////////////////
    // iterator SFINAE

    template <typename I>
    struct is_forward :
        boost::is_convertible<
            typename boost::iterator_traversal<I>::type,
            boost::forward_traversal_tag>
    {};

    template <typename I, typename ReturnType>
    struct enable_if_forward :
        boost::enable_if_c<
            boost::unordered::detail::is_forward<I>::value,
            ReturnType>
    {};

    template <typename I, typename ReturnType>
    struct disable_if_forward :
        boost::disable_if_c<
            boost::unordered::detail::is_forward<I>::value,
            ReturnType>
    {};

    ////////////////////////////////////////////////////////////////////////////
    // primes

#define BOOST_UNORDERED_PRIMES \
    (17ul)(29ul)(37ul)(53ul)(67ul)(79ul) \
    (97ul)(131ul)(193ul)(257ul)(389ul)(521ul)(769ul) \
    (1031ul)(1543ul)(2053ul)(3079ul)(6151ul)(12289ul)(24593ul) \
    (49157ul)(98317ul)(196613ul)(393241ul)(786433ul) \
    (1572869ul)(3145739ul)(6291469ul)(12582917ul)(25165843ul) \
    (50331653ul)(100663319ul)(201326611ul)(402653189ul)(805306457ul) \
    (1610612741ul)(3221225473ul)(4294967291ul)

    template<class T> struct prime_list_template
    {
        static std::size_t const value[];

#if !defined(SUNPRO_CC)
        static std::ptrdiff_t const length;
#else
        static std::ptrdiff_t const length
            = BOOST_PP_SEQ_SIZE(BOOST_UNORDERED_PRIMES);
#endif
    };

    template<class T>
    std::size_t const prime_list_template<T>::value[] = {
        BOOST_PP_SEQ_ENUM(BOOST_UNORDERED_PRIMES)
    };

#if !defined(SUNPRO_CC)
    template<class T>
    std::ptrdiff_t const prime_list_template<T>::length
        = BOOST_PP_SEQ_SIZE(BOOST_UNORDERED_PRIMES);
#endif

#undef BOOST_UNORDERED_PRIMES

    typedef prime_list_template<std::size_t> prime_list;

    // no throw
    inline std::size_t next_prime(std::size_t num) {
        std::size_t const* const prime_list_begin = prime_list::value;
        std::size_t const* const prime_list_end = prime_list_begin +
            prime_list::length;
        std::size_t const* bound =
            std::lower_bound(prime_list_begin, prime_list_end, num);
        if(bound == prime_list_end)
            bound--;
        return *bound;
    }

    // no throw
    inline std::size_t prev_prime(std::size_t num) {
        std::size_t const* const prime_list_begin = prime_list::value;
        std::size_t const* const prime_list_end = prime_list_begin +
            prime_list::length;
        std::size_t const* bound =
            std::upper_bound(prime_list_begin,prime_list_end, num);
        if(bound != prime_list_begin)
            bound--;
        return *bound;
    }

    ////////////////////////////////////////////////////////////////////////////
    // insert_size/initial_size

#if !defined(BOOST_NO_STD_DISTANCE)

    using ::std::distance;

#else

    template <class ForwardIterator>
    inline std::size_t distance(ForwardIterator i, ForwardIterator j) {
        std::size_t x;
        std::distance(i, j, x);
        return x;
    }

#endif

    template <class I>
    inline typename
        boost::unordered::detail::enable_if_forward<I, std::size_t>::type
        insert_size(I i, I j)
    {
        return std::distance(i, j);
    }

    template <class I>
    inline typename
        boost::unordered::detail::disable_if_forward<I, std::size_t>::type
        insert_size(I, I)
    {
        return 1;
    }

    template <class I>
    inline std::size_t initial_size(I i, I j,
        std::size_t num_buckets =
            boost::unordered::detail::default_bucket_count)
    {
        // TODO: Why +1?
        return (std::max)(
            boost::unordered::detail::insert_size(i, j) + 1,
            num_buckets);
    }

    ////////////////////////////////////////////////////////////////////////////
    // compressed

    template <typename T, int Index>
    struct compressed_base : private T
    {
        compressed_base(T const& x) : T(x) {}
        compressed_base(T& x, move_tag) : T(boost::move(x)) {}

        T& get() { return *this; }
        T const& get() const { return *this; }
    };

    template <typename T, int Index>
    struct uncompressed_base
    {
        uncompressed_base(T const& x) : value_(x) {}
        uncompressed_base(T& x, move_tag) : value_(boost::move(x)) {}

        T& get() { return value_; }
        T const& get() const { return value_; }
    private:
        T value_;
    };

    template <typename T, int Index>
    struct generate_base
      : boost::detail::if_true<
            boost::is_empty<T>::value
        >:: BOOST_NESTED_TEMPLATE then<
            boost::unordered::detail::compressed_base<T, Index>,
            boost::unordered::detail::uncompressed_base<T, Index>
        >
    {};

    template <typename T1, typename T2>
    struct compressed
      : private boost::unordered::detail::generate_base<T1, 1>::type,
        private boost::unordered::detail::generate_base<T2, 2>::type
    {
        typedef typename generate_base<T1, 1>::type base1;
        typedef typename generate_base<T2, 2>::type base2;

        typedef T1 first_type;
        typedef T2 second_type;

        first_type& first() {
            return static_cast<base1*>(this)->get();
        }

        first_type const& first() const {
            return static_cast<base1 const*>(this)->get();
        }

        second_type& second() {
            return static_cast<base2*>(this)->get();
        }

        second_type const& second() const {
            return static_cast<base2 const*>(this)->get();
        }

        template <typename First, typename Second>
        compressed(First const& x1, Second const& x2)
            : base1(x1), base2(x2) {}

        compressed(compressed const& x)
            : base1(x.first()), base2(x.second()) {}

        compressed(compressed& x, move_tag m)
            : base1(x.first(), m), base2(x.second(), m) {}

        void assign(compressed const& x)
        {
            first() = x.first();
            second() = x.second();
        }

        void move_assign(compressed& x)
        {
            first() = boost::move(x.first());
            second() = boost::move(x.second());
        }

        void swap(compressed& x)
        {
            boost::swap(first(), x.first());
            boost::swap(second(), x.second());
        }

    private:
        // Prevent assignment just to make use of assign or
        // move_assign explicit.
        compressed& operator=(compressed const&);
    };
}}}

namespace boost { namespace unordered { namespace detail {

    template <typename Types> struct table;
    template <typename NodePointer> struct bucket;
    struct ptr_bucket;
    template <typename Types> struct table_impl;
    template <typename Types> struct grouped_table_impl;

}}}

// The 'iterator_detail' namespace was a misguided attempt at avoiding ADL
// in the detail namespace. It didn't work because the template parameters
// were in detail. I'm not changing it at the moment to be safe. I might
// do in the future if I change the iterator types.
namespace boost { namespace unordered { namespace iterator_detail {

    ////////////////////////////////////////////////////////////////////////////
    // Iterators
    //
    // all no throw

    template <typename Node> struct iterator;
    template <typename Node, typename ConstNodePointer> struct c_iterator;
    template <typename Node, typename Policy> struct l_iterator;
    template <typename Node, typename ConstNodePointer, typename Policy>
        struct cl_iterator;

    // Local Iterators
    //
    // all no throw

    template <typename Node, typename Policy>
    struct l_iterator
        : public boost::iterator<
            std::forward_iterator_tag,
            typename Node::value_type,
            std::ptrdiff_t,
            typename Node::node_pointer,
            typename Node::value_type&>
    {
#if !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
        template <typename Node2, typename ConstNodePointer, typename Policy2>
        friend struct boost::unordered::iterator_detail::cl_iterator;
    private:
#endif
        typedef typename Node::node_pointer node_pointer;
        typedef boost::unordered::iterator_detail::iterator<Node> iterator;
        node_pointer ptr_;
        std::size_t bucket_;
        std::size_t bucket_count_;

    public:

        typedef typename Node::value_type value_type;

        l_iterator() BOOST_NOEXCEPT : ptr_() {}

        l_iterator(iterator x, std::size_t b, std::size_t c) BOOST_NOEXCEPT
            : ptr_(x.node_), bucket_(b), bucket_count_(c) {}

        value_type& operator*() const {
            return ptr_->value();
        }

        value_type* operator->() const {
            return ptr_->value_ptr();
        }

        l_iterator& operator++() {
            ptr_ = static_cast<node_pointer>(ptr_->next_);
            if (ptr_ && Policy::to_bucket(bucket_count_, ptr_->hash_)
                    != bucket_)
                ptr_ = node_pointer();
            return *this;
        }

        l_iterator operator++(int) {
            l_iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        bool operator==(l_iterator x) const BOOST_NOEXCEPT {
            return ptr_ == x.ptr_;
        }

        bool operator!=(l_iterator x) const BOOST_NOEXCEPT {
            return ptr_ != x.ptr_;
        }
    };

    template <typename Node, typename ConstNodePointer, typename Policy>
    struct cl_iterator
        : public boost::iterator<
            std::forward_iterator_tag,
            typename Node::value_type,
            std::ptrdiff_t,
            ConstNodePointer,
            typename Node::value_type const&>
    {
        friend struct boost::unordered::iterator_detail::l_iterator
            <Node, Policy>;
    private:

        typedef typename Node::node_pointer node_pointer;
        typedef boost::unordered::iterator_detail::iterator<Node> iterator;
        node_pointer ptr_;
        std::size_t bucket_;
        std::size_t bucket_count_;

    public:

        typedef typename Node::value_type value_type;

        cl_iterator() BOOST_NOEXCEPT : ptr_() {}

        cl_iterator(iterator x, std::size_t b, std::size_t c) BOOST_NOEXCEPT :
            ptr_(x.node_), bucket_(b), bucket_count_(c) {}

        cl_iterator(boost::unordered::iterator_detail::l_iterator<
                Node, Policy> const& x) BOOST_NOEXCEPT :
            ptr_(x.ptr_), bucket_(x.bucket_), bucket_count_(x.bucket_count_)
        {}

        value_type const& operator*() const {
            return ptr_->value();
        }

        value_type const* operator->() const {
            return ptr_->value_ptr();
        }

        cl_iterator& operator++() {
            ptr_ = static_cast<node_pointer>(ptr_->next_);
            if (ptr_ && Policy::to_bucket(bucket_count_, ptr_->hash_)
                    != bucket_)
                ptr_ = node_pointer();
            return *this;
        }

        cl_iterator operator++(int) {
            cl_iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        friend bool operator==(cl_iterator const& x, cl_iterator const& y)
            BOOST_NOEXCEPT
        {
            return x.ptr_ == y.ptr_;
        }

        friend bool operator!=(cl_iterator const& x, cl_iterator const& y)
            BOOST_NOEXCEPT
        {
            return x.ptr_ != y.ptr_;
        }
    };

    template <typename Node>
    struct iterator
        : public boost::iterator<
            std::forward_iterator_tag,
            typename Node::value_type,
            std::ptrdiff_t,
            typename Node::node_pointer,
            typename Node::value_type&>
    {
#if !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
        template <typename, typename>
        friend struct boost::unordered::iterator_detail::c_iterator;
        template <typename, typename>
        friend struct boost::unordered::iterator_detail::l_iterator;
        template <typename, typename, typename>
        friend struct boost::unordered::iterator_detail::cl_iterator;
        template <typename>
        friend struct boost::unordered::detail::table;
        template <typename>
        friend struct boost::unordered::detail::table_impl;
        template <typename>
        friend struct boost::unordered::detail::grouped_table_impl;
    private:
#endif
        typedef typename Node::node_pointer node_pointer;
        node_pointer node_;

    public:

        typedef typename Node::value_type value_type;

        iterator() BOOST_NOEXCEPT : node_() {}

        explicit iterator(typename Node::link_pointer x) BOOST_NOEXCEPT :
            node_(static_cast<node_pointer>(x)) {}

        value_type& operator*() const {
            return node_->value();
        }

        value_type* operator->() const {
            return &node_->value();
        }

        iterator& operator++() {
            node_ = static_cast<node_pointer>(node_->next_);
            return *this;
        }

        iterator operator++(int) {
            iterator tmp(node_);
            node_ = static_cast<node_pointer>(node_->next_);
            return tmp;
        }

        bool operator==(iterator const& x) const BOOST_NOEXCEPT {
            return node_ == x.node_;
        }

        bool operator!=(iterator const& x) const BOOST_NOEXCEPT {
            return node_ != x.node_;
        }
    };

    template <typename Node, typename ConstNodePointer>
    struct c_iterator
        : public boost::iterator<
            std::forward_iterator_tag,
            typename Node::value_type,
            std::ptrdiff_t,
            ConstNodePointer,
            typename Node::value_type const&>
    {
        friend struct boost::unordered::iterator_detail::iterator<Node>;

#if !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
        template <typename>
        friend struct boost::unordered::detail::table;
        template <typename>
        friend struct boost::unordered::detail::table_impl;
        template <typename>
        friend struct boost::unordered::detail::grouped_table_impl;

    private:
#endif
        typedef typename Node::node_pointer node_pointer;
        typedef boost::unordered::iterator_detail::iterator<Node> iterator;
        node_pointer node_;

    public:

        typedef typename Node::value_type value_type;

        c_iterator() BOOST_NOEXCEPT : node_() {}

        explicit c_iterator(typename Node::link_pointer x) BOOST_NOEXCEPT :
            node_(static_cast<node_pointer>(x)) {}

        c_iterator(iterator const& x) BOOST_NOEXCEPT : node_(x.node_) {}

        value_type const& operator*() const {
            return node_->value();
        }

        value_type const* operator->() const {
            return &node_->value();
        }

        c_iterator& operator++() {
            node_ = static_cast<node_pointer>(node_->next_);
            return *this;
        }

        c_iterator operator++(int) {
            c_iterator tmp(node_);
            node_ = static_cast<node_pointer>(node_->next_);
            return tmp;
        }

        friend bool operator==(c_iterator const& x, c_iterator const& y)
            BOOST_NOEXCEPT
        {
            return x.node_ == y.node_;
        }

        friend bool operator!=(c_iterator const& x, c_iterator const& y)
            BOOST_NOEXCEPT
        {
            return x.node_ != y.node_;
        }
    };
}}}

namespace boost { namespace unordered { namespace detail {

    ///////////////////////////////////////////////////////////////////
    //
    // Node construction

    template <typename NodeAlloc>
    struct node_constructor
    {
    private:

        typedef NodeAlloc node_allocator;
        typedef boost::unordered::detail::allocator_traits<NodeAlloc>
            node_allocator_traits;
        typedef typename node_allocator_traits::value_type node;
        typedef typename node_allocator_traits::pointer node_pointer;
        typedef typename node::value_type value_type;

    protected:

        node_allocator& alloc_;
        node_pointer node_;
        bool node_constructed_;
        bool value_constructed_;

    public:

        node_constructor(node_allocator& n) :
            alloc_(n),
            node_(),
            node_constructed_(false),
            value_constructed_(false)
        {
        }

        ~node_constructor();

        void construct();

        template <BOOST_UNORDERED_EMPLACE_TEMPLATE>
        void construct_with_value(BOOST_UNORDERED_EMPLACE_ARGS)
        {
            construct();
            boost::unordered::detail::func::construct_value_impl(
                alloc_, node_->value_ptr(), BOOST_UNORDERED_EMPLACE_FORWARD);
            value_constructed_ = true;
        }

        template <typename A0>
        void construct_with_value2(BOOST_FWD_REF(A0) a0)
        {
            construct();
            boost::unordered::detail::func::construct_value_impl(
                alloc_, node_->value_ptr(),
                BOOST_UNORDERED_EMPLACE_ARGS1(boost::forward<A0>(a0)));
            value_constructed_ = true;
        }

        value_type const& value() const {
            BOOST_ASSERT(node_ && node_constructed_ && value_constructed_);
            return node_->value();
        }

        // no throw
        node_pointer release()
        {
            BOOST_ASSERT(node_ && node_constructed_);
            node_pointer p = node_;
            node_ = node_pointer();
            return p;
        }

    private:
        node_constructor(node_constructor const&);
        node_constructor& operator=(node_constructor const&);
    };

    template <typename Alloc>
    node_constructor<Alloc>::~node_constructor()
    {
        if (node_) {
            if (value_constructed_) {
                boost::unordered::detail::func::destroy_value_impl(alloc_,
                    node_->value_ptr());
            }

            if (node_constructed_) {
                boost::unordered::detail::func::destroy(
                    boost::addressof(*node_));
            }

            node_allocator_traits::deallocate(alloc_, node_, 1);
        }
    }

    template <typename Alloc>
    void node_constructor<Alloc>::construct()
    {
        if(!node_) {
            node_constructed_ = false;
            value_constructed_ = false;

            node_ = node_allocator_traits::allocate(alloc_, 1);

            new ((void*) boost::addressof(*node_)) node();
            node_->init(node_);
            node_constructed_ = true;
        }
        else {
            BOOST_ASSERT(node_constructed_);

            if (value_constructed_)
            {
                boost::unordered::detail::func::destroy_value_impl(alloc_,
                    node_->value_ptr());
                value_constructed_ = false;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////
    //
    // Node Holder
    //
    // Temporary store for nodes. Deletes any that aren't used.

    template <typename NodeAlloc>
    struct node_holder : private node_constructor<NodeAlloc>
    {
    private:
        typedef node_constructor<NodeAlloc> base;

        typedef NodeAlloc node_allocator;
        typedef boost::unordered::detail::allocator_traits<NodeAlloc>
            node_allocator_traits;
        typedef typename node_allocator_traits::value_type node;
        typedef typename node_allocator_traits::pointer node_pointer;
        typedef typename node::value_type value_type;
        typedef typename node::link_pointer link_pointer;
        typedef boost::unordered::iterator_detail::iterator<node> iterator;

        node_pointer nodes_;

    public:

        template <typename Table>
        explicit node_holder(Table& b) :
            base(b.node_alloc()),
            nodes_()
        {
            if (b.size_) {
                typename Table::link_pointer prev = b.get_previous_start();
                nodes_ = static_cast<node_pointer>(prev->next_);
                prev->next_ = link_pointer();
                b.size_ = 0;
            }
        }

        ~node_holder();

        void node_for_assignment()
        {
            if (!this->node_ && nodes_) {
                this->node_ = nodes_;
                nodes_ = static_cast<node_pointer>(nodes_->next_);
                this->node_->init(this->node_);
                this->node_->next_ = link_pointer();

                this->node_constructed_ = true;
                this->value_constructed_ = true;
            }
        }

        template <typename T>
        inline void assign_impl(T const& v) {
            if (this->node_ && this->value_constructed_) {
                this->node_->value() = v;
            }
            else {
                this->construct_with_value2(v);
            }
        }

        template <typename T1, typename T2>
        inline void assign_impl(std::pair<T1 const, T2> const& v) {
            this->construct_with_value2(v);
        }

        template <typename T>
        inline void move_assign_impl(T& v) {
            if (this->node_ && this->value_constructed_) {
                this->node_->value() = boost::move(v);
            }
            else {
                this->construct_with_value2(boost::move(v));
            }
        }

        template <typename T1, typename T2>
        inline void move_assign_impl(std::pair<T1 const, T2>& v) {
            this->construct_with_value2(boost::move(v));
        }

        node_pointer copy_of(value_type const& v)
        {
            node_for_assignment();
            assign_impl(v);
            return base::release();
        }

        node_pointer move_copy_of(value_type& v)
        {
            node_for_assignment();
            move_assign_impl(v);
            return base::release();
        }

        iterator begin() const
        {
            return iterator(nodes_);
        }
    };

    template <typename Alloc>
    node_holder<Alloc>::~node_holder()
    {
        while (nodes_) {
            node_pointer p = nodes_;
            nodes_ = static_cast<node_pointer>(p->next_);

            boost::unordered::detail::func::destroy_value_impl(this->alloc_,
                p->value_ptr());
            boost::unordered::detail::func::destroy(boost::addressof(*p));
            node_allocator_traits::deallocate(this->alloc_, p, 1);
        }
    }

    ///////////////////////////////////////////////////////////////////
    //
    // Bucket

    template <typename NodePointer>
    struct bucket
    {
        typedef NodePointer link_pointer;
        link_pointer next_;

        bucket() : next_() {}

        link_pointer first_from_start()
        {
            return next_;
        }

        enum { extra_node = true };
    };

    struct ptr_bucket
    {
        typedef ptr_bucket* link_pointer;
        link_pointer next_;

        ptr_bucket() : next_(0) {}

        link_pointer first_from_start()
        {
            return this;
        }

        enum { extra_node = false };
    };

    ///////////////////////////////////////////////////////////////////
    //
    // Hash Policy

    template <typename SizeT>
    struct prime_policy
    {
        template <typename Hash, typename T>
        static inline SizeT apply_hash(Hash const& hf, T const& x) {
            return hf(x);
        }

        static inline SizeT to_bucket(SizeT bucket_count, SizeT hash) {
            return hash % bucket_count;
        }

        static inline SizeT new_bucket_count(SizeT min) {
            return boost::unordered::detail::next_prime(min);
        }

        static inline SizeT prev_bucket_count(SizeT max) {
            return boost::unordered::detail::prev_prime(max);
        }
    };

    template <typename SizeT>
    struct mix64_policy
    {
        template <typename Hash, typename T>
        static inline SizeT apply_hash(Hash const& hf, T const& x) {
            SizeT key = hf(x);
            key = (~key) + (key << 21); // key = (key << 21) - key - 1;
            key = key ^ (key >> 24);
            key = (key + (key << 3)) + (key << 8); // key * 265
            key = key ^ (key >> 14);
            key = (key + (key << 2)) + (key << 4); // key * 21
            key = key ^ (key >> 28);
            key = key + (key << 31);
            return key;
        }

        static inline SizeT to_bucket(SizeT bucket_count, SizeT hash) {
            return hash & (bucket_count - 1);
        }

        static inline SizeT new_bucket_count(SizeT min) {
            if (min <= 4) return 4;
            --min;
            min |= min >> 1;
            min |= min >> 2;
            min |= min >> 4;
            min |= min >> 8;
            min |= min >> 16;
            min |= min >> 32;
            return min + 1;
        }

        static inline SizeT prev_bucket_count(SizeT max) {
            max |= max >> 1;
            max |= max >> 2;
            max |= max >> 4;
            max |= max >> 8;
            max |= max >> 16;
            max |= max >> 32;
            return (max >> 1) + 1;
        }
    };

    template <int digits, int radix>
    struct pick_policy_impl {
        typedef prime_policy<std::size_t> type;
    };

    template <>
    struct pick_policy_impl<64, 2> {
        typedef mix64_policy<std::size_t> type;
    };

    template <typename T>
    struct pick_policy :
        pick_policy_impl<
            std::numeric_limits<std::size_t>::digits,
            std::numeric_limits<std::size_t>::radix> {};

    // While the mix policy is generally faster, the prime policy is a lot
    // faster when a large number consecutive integers are used, because
    // there are no collisions. Since that is probably quite common, use
    // prime policy for integeral types. But not the smaller ones, as they
    // don't have enough unique values for this to be an issue.

    template <>
    struct pick_policy<int> {
        typedef prime_policy<std::size_t> type;
    };

    template <>
    struct pick_policy<unsigned int> {
        typedef prime_policy<std::size_t> type;
    };

    template <>
    struct pick_policy<long> {
        typedef prime_policy<std::size_t> type;
    };

    template <>
    struct pick_policy<unsigned long> {
        typedef prime_policy<std::size_t> type;
    };

    // TODO: Maybe not if std::size_t is smaller than long long.
#if !defined(BOOST_NO_LONG_LONG)
    template <>
    struct pick_policy<long long> {
        typedef prime_policy<std::size_t> type;
    };

    template <>
    struct pick_policy<unsigned long long> {
        typedef prime_policy<std::size_t> type;
    };
#endif

    ////////////////////////////////////////////////////////////////////////////
    // Functions

    // Assigning and swapping the equality and hash function objects
    // needs strong exception safety. To implement that normally we'd
    // require one of them to be known to not throw and the other to
    // guarantee strong exception safety. Unfortunately they both only
    // have basic exception safety. So to acheive strong exception
    // safety we have storage space for two copies, and assign the new
    // copies to the unused space. Then switch to using that to use
    // them. This is implemented in 'set_hash_functions' which
    // atomically assigns the new function objects in a strongly
    // exception safe manner.

    template <class H, class P, bool NoThrowMoveAssign>
    class set_hash_functions;

    template <class H, class P>
    class functions
    {
    public:
        static const bool nothrow_move_assignable =
                boost::is_nothrow_move_assignable<H>::value &&
                boost::is_nothrow_move_assignable<P>::value;
        static const bool nothrow_move_constructible =
                boost::is_nothrow_move_constructible<H>::value &&
                boost::is_nothrow_move_constructible<P>::value;

    private:
        friend class boost::unordered::detail::set_hash_functions<H, P,
               nothrow_move_assignable>;
        functions& operator=(functions const&);

        typedef compressed<H, P> function_pair;

        typedef typename boost::aligned_storage<
            sizeof(function_pair),
            boost::alignment_of<function_pair>::value>::type aligned_function;

        bool current_; // The currently active functions.
        aligned_function funcs_[2];

        function_pair const& current() const {
            return *static_cast<function_pair const*>(
                static_cast<void const*>(&funcs_[current_]));
        }

        function_pair& current() {
            return *static_cast<function_pair*>(
                static_cast<void*>(&funcs_[current_]));
        }

        void construct(bool which, H const& hf, P const& eq)
        {
            new((void*) &funcs_[which]) function_pair(hf, eq);
        }

        void construct(bool which, function_pair const& f,
                boost::unordered::detail::false_type =
                    boost::unordered::detail::false_type())
        {
            new((void*) &funcs_[which]) function_pair(f);
        }

        void construct(bool which, function_pair& f,
                boost::unordered::detail::true_type)
        {
            new((void*) &funcs_[which]) function_pair(f,
                boost::unordered::detail::move_tag());
        }

        void destroy(bool which)
        {
            boost::unordered::detail::func::destroy((function_pair*)(&funcs_[which]));
        }

    public:

        typedef boost::unordered::detail::set_hash_functions<H, P,
                nothrow_move_assignable> set_hash_functions;

        functions(H const& hf, P const& eq)
            : current_(false)
        {
            construct(current_, hf, eq);
        }

        functions(functions const& bf)
            : current_(false)
        {
            construct(current_, bf.current());
        }

        functions(functions& bf, boost::unordered::detail::move_tag)
            : current_(false)
        {
            construct(current_, bf.current(),
                boost::unordered::detail::integral_constant<bool,
                    nothrow_move_constructible>());
        }

        ~functions() {
            this->destroy(current_);
        }

        H const& hash_function() const {
            return current().first();
        }

        P const& key_eq() const {
            return current().second();
        }
    };

    template <class H, class P>
    class set_hash_functions<H, P, false>
    {
        set_hash_functions(set_hash_functions const&);
        set_hash_functions& operator=(set_hash_functions const&);

        typedef functions<H, P> functions_type;

        functions_type& functions_;
        bool tmp_functions_;

    public:

        set_hash_functions(functions_type& f, H const& h, P const& p)
          : functions_(f),
            tmp_functions_(!f.current_)
        {
            f.construct(tmp_functions_, h, p);
        }

        set_hash_functions(functions_type& f, functions_type const& other)
          : functions_(f),
            tmp_functions_(!f.current_)
        {
            f.construct(tmp_functions_, other.current());
        }

        ~set_hash_functions()
        {
            functions_.destroy(tmp_functions_);
        }

        void commit()
        {
            functions_.current_ = tmp_functions_;
            tmp_functions_ = !tmp_functions_;
        }
    };

    template <class H, class P>
    class set_hash_functions<H, P, true>
    {
        set_hash_functions(set_hash_functions const&);
        set_hash_functions& operator=(set_hash_functions const&);

        typedef functions<H, P> functions_type;

        functions_type& functions_;
        H hash_;
        P pred_;

    public:

        set_hash_functions(functions_type& f, H const& h, P const& p) :
            functions_(f),
            hash_(h),
            pred_(p) {}

        set_hash_functions(functions_type& f, functions_type const& other) :
            functions_(f),
            hash_(other.hash_function()),
            pred_(other.key_eq()) {}

        void commit()
        {
            functions_.current().first() = boost::move(hash_);
            functions_.current().second() = boost::move(pred_);
        }
    };

    ////////////////////////////////////////////////////////////////////////////
    // rvalue parameters when type can't be a BOOST_RV_REF(T) parameter
    // e.g. for int

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
#   define BOOST_UNORDERED_RV_REF(T) BOOST_RV_REF(T)
#else
    struct please_ignore_this_overload {
        typedef please_ignore_this_overload type;
    };

    template <typename T>
    struct rv_ref_impl {
        typedef BOOST_RV_REF(T) type;
    };

    template <typename T>
    struct rv_ref :
        boost::detail::if_true<
            boost::is_class<T>::value
        >::BOOST_NESTED_TEMPLATE then <
            boost::unordered::detail::rv_ref_impl<T>,
            please_ignore_this_overload
        >::type
    {};

#   define BOOST_UNORDERED_RV_REF(T) \
        typename boost::unordered::detail::rv_ref<T>::type
#endif

    ////////////////////////////////////////////////////////////////////////////
    // convert double to std::size_t

    inline std::size_t double_to_size(double f)
    {
        return f >= static_cast<double>(
            (std::numeric_limits<std::size_t>::max)()) ?
            (std::numeric_limits<std::size_t>::max)() :
            static_cast<std::size_t>(f);
    }

    // The space used to store values in a node.

    template <typename ValueType>
    struct value_base
    {
        typedef ValueType value_type;

        typename boost::aligned_storage<
            sizeof(value_type),
            boost::alignment_of<value_type>::value>::type data_;

        void* address() {
            return this;
        }

        value_type& value() {
            return *(ValueType*) this;
        }

        value_type* value_ptr() {
            return (ValueType*) this;
        }

    private:

        value_base& operator=(value_base const&);
    };

    template <typename NodeAlloc>
    struct copy_nodes
    {
        typedef boost::unordered::detail::allocator_traits<NodeAlloc>
            node_allocator_traits;

        node_constructor<NodeAlloc> constructor;

        explicit copy_nodes(NodeAlloc& a) : constructor(a) {}

        typename node_allocator_traits::pointer create(
                typename node_allocator_traits::value_type::value_type const& v)
        {
            constructor.construct_with_value2(v);
            return constructor.release();
        }
    };

    template <typename NodeAlloc>
    struct move_nodes
    {
        typedef boost::unordered::detail::allocator_traits<NodeAlloc>
            node_allocator_traits;

        node_constructor<NodeAlloc> constructor;

        explicit move_nodes(NodeAlloc& a) : constructor(a) {}

        typename node_allocator_traits::pointer create(
                typename node_allocator_traits::value_type::value_type& v)
        {
            constructor.construct_with_value2(boost::move(v));
            return constructor.release();
        }
    };

    template <typename Buckets>
    struct assign_nodes
    {
        node_holder<typename Buckets::node_allocator> holder;

        explicit assign_nodes(Buckets& b) : holder(b) {}

        typename Buckets::node_pointer create(
                typename Buckets::value_type const& v)
        {
            return holder.copy_of(v);
        }
    };

    template <typename Buckets>
    struct move_assign_nodes
    {
        node_holder<typename Buckets::node_allocator> holder;

        explicit move_assign_nodes(Buckets& b) : holder(b) {}

        typename Buckets::node_pointer create(
                typename Buckets::value_type& v)
        {
            return holder.move_copy_of(v);
        }
    };

    template <typename Types>
    struct table :
        boost::unordered::detail::functions<
            typename Types::hasher,
            typename Types::key_equal>
    {
    private:
        table(table const&);
        table& operator=(table const&);
    public:
        typedef typename Types::node node;
        typedef typename Types::bucket bucket;
        typedef typename Types::hasher hasher;
        typedef typename Types::key_equal key_equal;
        typedef typename Types::key_type key_type;
        typedef typename Types::extractor extractor;
        typedef typename Types::value_type value_type;
        typedef typename Types::table table_impl;
        typedef typename Types::link_pointer link_pointer;
        typedef typename Types::policy policy;

        typedef boost::unordered::detail::functions<
            typename Types::hasher,
            typename Types::key_equal> functions;
        typedef typename functions::set_hash_functions set_hash_functions;

        typedef typename Types::allocator allocator;
        typedef typename boost::unordered::detail::
            rebind_wrap<allocator, node>::type node_allocator;
        typedef typename boost::unordered::detail::
            rebind_wrap<allocator, bucket>::type bucket_allocator;
        typedef boost::unordered::detail::allocator_traits<node_allocator>
            node_allocator_traits;
        typedef boost::unordered::detail::allocator_traits<bucket_allocator>
            bucket_allocator_traits;
        typedef typename node_allocator_traits::pointer
            node_pointer;
        typedef typename node_allocator_traits::const_pointer
            const_node_pointer;
        typedef typename bucket_allocator_traits::pointer
            bucket_pointer;
        typedef boost::unordered::detail::node_constructor<node_allocator>
            node_constructor;

        typedef boost::unordered::iterator_detail::
            iterator<node> iterator;
        typedef boost::unordered::iterator_detail::
            c_iterator<node, const_node_pointer> c_iterator;
        typedef boost::unordered::iterator_detail::
            l_iterator<node, policy> l_iterator;
        typedef boost::unordered::iterator_detail::
            cl_iterator<node, const_node_pointer, policy> cl_iterator;

        ////////////////////////////////////////////////////////////////////////
        // Members

        boost::unordered::detail::compressed<bucket_allocator, node_allocator>
            allocators_;
        std::size_t bucket_count_;
        std::size_t size_;
        float mlf_;
        std::size_t max_load_;
        bucket_pointer buckets_;

        ////////////////////////////////////////////////////////////////////////
        // Data access

        bucket_allocator const& bucket_alloc() const
        {
            return allocators_.first();
        }

        node_allocator const& node_alloc() const
        {
            return allocators_.second();
        }

        bucket_allocator& bucket_alloc()
        {
            return allocators_.first();
        }

        node_allocator& node_alloc()
        {
            return allocators_.second();
        }

        std::size_t max_bucket_count() const
        {
            // -1 to account for the start bucket.
            return policy::prev_bucket_count(
                bucket_allocator_traits::max_size(bucket_alloc()) - 1);
        }

        bucket_pointer get_bucket(std::size_t bucket_index) const
        {
            BOOST_ASSERT(buckets_);
            return buckets_ + static_cast<std::ptrdiff_t>(bucket_index);
        }

        link_pointer get_previous_start() const
        {
            return get_bucket(bucket_count_)->first_from_start();
        }

        link_pointer get_previous_start(std::size_t bucket_index) const
        {
            return get_bucket(bucket_index)->next_;
        }

        iterator begin() const
        {
            return size_ ? iterator(get_previous_start()->next_) : iterator();
        }

        iterator begin(std::size_t bucket_index) const
        {
            if (!size_) return iterator();
            link_pointer prev = get_previous_start(bucket_index);
            return prev ? iterator(prev->next_) : iterator();
        }
        
        std::size_t hash_to_bucket(std::size_t hash_value) const
        {
            return policy::to_bucket(bucket_count_, hash_value);
        }

        float load_factor() const
        {
            BOOST_ASSERT(bucket_count_ != 0);
            return static_cast<float>(size_)
                / static_cast<float>(bucket_count_);
        }

        std::size_t bucket_size(std::size_t index) const
        {
            iterator it = begin(index);
            if (!it.node_) return 0;

            std::size_t count = 0;
            while(it.node_ && hash_to_bucket(it.node_->hash_) == index)
            {
                ++count;
                ++it;
            }

            return count;
        }

        ////////////////////////////////////////////////////////////////////////
        // Load methods

        std::size_t max_size() const
        {
            using namespace std;
    
            // size < mlf_ * count
            return boost::unordered::detail::double_to_size(ceil(
                    static_cast<double>(mlf_) *
                    static_cast<double>(max_bucket_count())
                )) - 1;
        }

        void recalculate_max_load()
        {
            using namespace std;
    
            // From 6.3.1/13:
            // Only resize when size >= mlf_ * count
            max_load_ = buckets_ ? boost::unordered::detail::double_to_size(ceil(
                    static_cast<double>(mlf_) *
                    static_cast<double>(bucket_count_)
                )) : 0;

        }

        void max_load_factor(float z)
        {
            BOOST_ASSERT(z > 0);
            mlf_ = (std::max)(z, minimum_max_load_factor);
            recalculate_max_load();
        }

        std::size_t min_buckets_for_size(std::size_t size) const
        {
            BOOST_ASSERT(mlf_ >= minimum_max_load_factor);
    
            using namespace std;
    
            // From 6.3.1/13:
            // size < mlf_ * count
            // => count > size / mlf_
            //
            // Or from rehash post-condition:
            // count > size / mlf_

            return policy::new_bucket_count(
                boost::unordered::detail::double_to_size(floor(
                    static_cast<double>(size) /
                    static_cast<double>(mlf_))) + 1);
        }

        ////////////////////////////////////////////////////////////////////////
        // Constructors

        table(std::size_t num_buckets,
                hasher const& hf,
                key_equal const& eq,
                node_allocator const& a) :
            functions(hf, eq),
            allocators_(a,a),
            bucket_count_(policy::new_bucket_count(num_buckets)),
            size_(0),
            mlf_(1.0f),
            max_load_(0),
            buckets_()
        {}

        table(table const& x, node_allocator const& a) :
            functions(x),
            allocators_(a,a),
            bucket_count_(x.min_buckets_for_size(x.size_)),
            size_(0),
            mlf_(x.mlf_),
            max_load_(0),
            buckets_()
        {}

        table(table& x, boost::unordered::detail::move_tag m) :
            functions(x, m),
            allocators_(x.allocators_, m),
            bucket_count_(x.bucket_count_),
            size_(x.size_),
            mlf_(x.mlf_),
            max_load_(x.max_load_),
            buckets_(x.buckets_)
        {
            x.buckets_ = bucket_pointer();
            x.size_ = 0;
            x.max_load_ = 0;
        }

        table(table& x, node_allocator const& a,
                boost::unordered::detail::move_tag m) :
            functions(x, m),
            allocators_(a, a),
            bucket_count_(x.bucket_count_),
            size_(0),
            mlf_(x.mlf_),
            max_load_(x.max_load_),
            buckets_()
        {}

        ////////////////////////////////////////////////////////////////////////
        // Initialisation.

        void init(table const& x)
        {
            if (x.size_) {
                create_buckets(bucket_count_);
                copy_nodes<node_allocator> node_creator(node_alloc());
                table_impl::fill_buckets(x.begin(), *this, node_creator);
            }
        }

        void move_init(table& x)
        {
            if(node_alloc() == x.node_alloc()) {
                move_buckets_from(x);
            }
            else if(x.size_) {
                // TODO: Could pick new bucket size?
                create_buckets(bucket_count_);

                move_nodes<node_allocator> node_creator(node_alloc());
                node_holder<node_allocator> nodes(x);
                table_impl::fill_buckets(nodes.begin(), *this, node_creator);
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // Create buckets

        void create_buckets(std::size_t new_count)
        {
            boost::unordered::detail::array_constructor<bucket_allocator>
                constructor(bucket_alloc());
    
            // Creates an extra bucket to act as the start node.
            constructor.construct(bucket(), new_count + 1);

            if (buckets_)
            {
                // Copy the nodes to the new buckets, including the dummy
                // node if there is one.
                (constructor.get() +
                    static_cast<std::ptrdiff_t>(new_count))->next_ =
                        (buckets_ + static_cast<std::ptrdiff_t>(
                            bucket_count_))->next_;
                destroy_buckets();
            }
            else if (bucket::extra_node)
            {
                node_constructor a(node_alloc());
                a.construct();

                (constructor.get() +
                    static_cast<std::ptrdiff_t>(new_count))->next_ =
                        a.release();
            }

            bucket_count_ = new_count;
            buckets_ = constructor.release();
            recalculate_max_load();
        }

        ////////////////////////////////////////////////////////////////////////
        // Swap and Move

        void swap_allocators(table& other, false_type)
        {
            boost::unordered::detail::func::ignore_unused_variable_warning(other);

            // According to 23.2.1.8, if propagate_on_container_swap is
            // false the behaviour is undefined unless the allocators
            // are equal.
            BOOST_ASSERT(node_alloc() == other.node_alloc());
        }

        void swap_allocators(table& other, true_type)
        {
            allocators_.swap(other.allocators_);
        }

        // Only swaps the allocators if propagate_on_container_swap
        void swap(table& x)
        {
            set_hash_functions op1(*this, x);
            set_hash_functions op2(x, *this);

            // I think swap can throw if Propagate::value,
            // since the allocators' swap can throw. Not sure though.
            swap_allocators(x,
                boost::unordered::detail::integral_constant<bool,
                    allocator_traits<node_allocator>::
                    propagate_on_container_swap::value>());

            boost::swap(buckets_, x.buckets_);
            boost::swap(bucket_count_, x.bucket_count_);
            boost::swap(size_, x.size_);
            std::swap(mlf_, x.mlf_);
            std::swap(max_load_, x.max_load_);
            op1.commit();
            op2.commit();
        }

        void move_buckets_from(table& other)
        {
            BOOST_ASSERT(node_alloc() == other.node_alloc());
            BOOST_ASSERT(!buckets_);
            buckets_ = other.buckets_;
            bucket_count_ = other.bucket_count_;
            size_ = other.size_;
            other.buckets_ = bucket_pointer();
            other.size_ = 0;
            other.max_load_ = 0;
        }

        ////////////////////////////////////////////////////////////////////////
        // Delete/destruct

        ~table()
        {
            delete_buckets();
        }

        void delete_node(link_pointer prev)
        {
            node_pointer n = static_cast<node_pointer>(prev->next_);
            prev->next_ = n->next_;

            boost::unordered::detail::func::destroy_value_impl(node_alloc(),
                n->value_ptr());
            boost::unordered::detail::func::destroy(boost::addressof(*n));
            node_allocator_traits::deallocate(node_alloc(), n, 1);
            --size_;
        }

        std::size_t delete_nodes(link_pointer prev, link_pointer end)
        {
            BOOST_ASSERT(prev->next_ != end);

            std::size_t count = 0;

            do {
                delete_node(prev);
                ++count;
            } while (prev->next_ != end);

            return count;
        }

        void delete_buckets()
        {
            if(buckets_) {
                if (size_) delete_nodes(get_previous_start(), link_pointer());

                if (bucket::extra_node) {
                    node_pointer n = static_cast<node_pointer>(
                            get_bucket(bucket_count_)->next_);
                    boost::unordered::detail::func::destroy(
                            boost::addressof(*n));
                    node_allocator_traits::deallocate(node_alloc(), n, 1);
                }

                destroy_buckets();
                buckets_ = bucket_pointer();
                max_load_ = 0;
            }

            BOOST_ASSERT(!size_);
        }

        void clear()
        {
            if (!size_) return;

            delete_nodes(get_previous_start(), link_pointer());
            clear_buckets();

            BOOST_ASSERT(!size_);
        }

        void clear_buckets()
        {
            bucket_pointer end = get_bucket(bucket_count_);
            for(bucket_pointer it = buckets_; it != end; ++it)
            {
                it->next_ = node_pointer();
            }
        }

        void destroy_buckets()
        {
            bucket_pointer end = get_bucket(bucket_count_ + 1);
            for(bucket_pointer it = buckets_; it != end; ++it)
            {
                boost::unordered::detail::func::destroy(
                    boost::addressof(*it));
            }

            bucket_allocator_traits::deallocate(bucket_alloc(),
                buckets_, bucket_count_ + 1);
        }

        ////////////////////////////////////////////////////////////////////////
        // Fix buckets after delete
        //

        std::size_t fix_bucket(std::size_t bucket_index, link_pointer prev)
        {
            link_pointer end = prev->next_;
            std::size_t bucket_index2 = bucket_index;

            if (end)
            {
                bucket_index2 = hash_to_bucket(
                    static_cast<node_pointer>(end)->hash_);

                // If begin and end are in the same bucket, then
                // there's nothing to do.
                if (bucket_index == bucket_index2) return bucket_index2;

                // Update the bucket containing end.
                get_bucket(bucket_index2)->next_ = prev;
            }

            // Check if this bucket is now empty.
            bucket_pointer this_bucket = get_bucket(bucket_index);
            if (this_bucket->next_ == prev)
                this_bucket->next_ = link_pointer();

            return bucket_index2;
        }

        ////////////////////////////////////////////////////////////////////////
        // Assignment

        void assign(table const& x)
        {
            if (this != boost::addressof(x))
            {
                assign(x,
                    boost::unordered::detail::integral_constant<bool,
                        allocator_traits<node_allocator>::
                        propagate_on_container_copy_assignment::value>());
            }
        }

        void assign(table const& x, false_type)
        {
            // Strong exception safety.
            set_hash_functions new_func_this(*this, x);
            new_func_this.commit();
            mlf_ = x.mlf_;
            recalculate_max_load();

            if (!size_ && !x.size_) return;

            if (x.size_ >= max_load_) {
                create_buckets(min_buckets_for_size(x.size_));
            }
            else {
                clear_buckets();
            }

            // assign_nodes takes ownership of the container's elements,
            // assigning to them if possible, and deleting any that are
            // left over.
            assign_nodes<table> node_creator(*this);
            table_impl::fill_buckets(x.begin(), *this, node_creator);
        }

        void assign(table const& x, true_type)
        {
            if (node_alloc() == x.node_alloc()) {
                allocators_.assign(x.allocators_);
                assign(x, false_type());
            }
            else {
                set_hash_functions new_func_this(*this, x);

                // Delete everything with current allocators before assigning
                // the new ones.
                delete_buckets();
                allocators_.assign(x.allocators_);

                // Copy over other data, all no throw.
                new_func_this.commit();
                mlf_ = x.mlf_;
                bucket_count_ = min_buckets_for_size(x.size_);
                max_load_ = 0;

                // Finally copy the elements.
                if (x.size_) {
                    create_buckets(bucket_count_);
                    copy_nodes<node_allocator> node_creator(node_alloc());
                    table_impl::fill_buckets(x.begin(), *this, node_creator);
                }
            }
        }

        void move_assign(table& x)
        {
            if (this != boost::addressof(x))
            {
                move_assign(x,
                    boost::unordered::detail::integral_constant<bool,
                        allocator_traits<node_allocator>::
                        propagate_on_container_move_assignment::value>());
            }
        }

        void move_assign(table& x, true_type)
        {
            delete_buckets();
            allocators_.move_assign(x.allocators_);
            move_assign_no_alloc(x);
        }

        void move_assign(table& x, false_type)
        {
            if (node_alloc() == x.node_alloc()) {
                delete_buckets();
                move_assign_no_alloc(x);
            }
            else {
                set_hash_functions new_func_this(*this, x);
                new_func_this.commit();
                mlf_ = x.mlf_;
                recalculate_max_load();

                if (!size_ && !x.size_) return;

                if (x.size_ >= max_load_) {
                    create_buckets(min_buckets_for_size(x.size_));
                }
                else {
                    clear_buckets();
                }

                // move_assign_nodes takes ownership of the container's
                // elements, assigning to them if possible, and deleting
                // any that are left over.
                move_assign_nodes<table> node_creator(*this);
                node_holder<node_allocator> nodes(x);
                table_impl::fill_buckets(nodes.begin(), *this, node_creator);
            }
        }
        
        void move_assign_no_alloc(table& x)
        {
            set_hash_functions new_func_this(*this, x);
            // No throw from here.
            mlf_ = x.mlf_;
            max_load_ = x.max_load_;
            move_buckets_from(x);
            new_func_this.commit();
        }

        // Accessors

        key_type const& get_key(value_type const& x) const
        {
            return extractor::extract(x);
        }

        std::size_t hash(key_type const& k) const
        {
            return policy::apply_hash(this->hash_function(), k);
        }

        // Find Node

        template <typename Key, typename Hash, typename Pred>
        iterator generic_find_node(
                Key const& k,
                Hash const& hf,
                Pred const& eq) const
        {
            return static_cast<table_impl const*>(this)->
                find_node_impl(policy::apply_hash(hf, k), k, eq);
        }

        iterator find_node(
                std::size_t key_hash,
                key_type const& k) const
        {
            return static_cast<table_impl const*>(this)->
                find_node_impl(key_hash, k, this->key_eq());
        }

        iterator find_node(key_type const& k) const
        {
            return static_cast<table_impl const*>(this)->
                find_node_impl(hash(k), k, this->key_eq());
        }

        iterator find_matching_node(iterator n) const
        {
            // TODO: Does this apply to C++11?
            //
            // For some stupid reason, I decided to support equality comparison
            // when different hash functions are used. So I can't use the hash
            // value from the node here.
    
            return find_node(get_key(*n));
        }

        // Reserve and rehash

        void reserve_for_insert(std::size_t);
        void rehash(std::size_t);
        void reserve(std::size_t);
    };

    ////////////////////////////////////////////////////////////////////////////
    // Reserve & Rehash

    // basic exception safety
    template <typename Types>
    inline void table<Types>::reserve_for_insert(std::size_t size)
    {
        if (!buckets_) {
            create_buckets((std::max)(bucket_count_,
                min_buckets_for_size(size)));
        }
        // According to the standard this should be 'size >= max_load_',
        // but I think this is better, defect report filed.
        else if(size > max_load_) {
            std::size_t num_buckets
                = min_buckets_for_size((std::max)(size,
                    size_ + (size_ >> 1)));

            if (num_buckets != bucket_count_)
                static_cast<table_impl*>(this)->rehash_impl(num_buckets);
        }
    }

    // if hash function throws, basic exception safety
    // strong otherwise.

    template <typename Types>
    inline void table<Types>::rehash(std::size_t min_buckets)
    {
        using namespace std;

        if(!size_) {
            delete_buckets();
            bucket_count_ = policy::new_bucket_count(min_buckets);
        }
        else {
            min_buckets = policy::new_bucket_count((std::max)(min_buckets,
                boost::unordered::detail::double_to_size(floor(
                    static_cast<double>(size_) /
                    static_cast<double>(mlf_))) + 1));

            if(min_buckets != bucket_count_)
                static_cast<table_impl*>(this)->rehash_impl(min_buckets);
        }
    }

    template <typename Types>
    inline void table<Types>::reserve(std::size_t num_elements)
    {
        rehash(static_cast<std::size_t>(
            std::ceil(static_cast<double>(num_elements) / mlf_)));
    }
}}}

#if defined(BOOST_MSVC)
#pragma warning(pop)
#endif

#endif
