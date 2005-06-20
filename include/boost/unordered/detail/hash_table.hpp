// JTC1/SC22/WG21 N1456 Hash table implementation
// http://std.dkuug.dk/jtc1/sc22/wg21/docs/papers/2003/n1456.html

// boost/detail/hash_table.hpp

// Copyright © 2003-2004 Jeremy B. Maitin-Shepard.
// Copyright © 2005 Daniel James

// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy
// at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNORDERED_DETAIL_HASH_TABLE_HPP_INCLUDED
#define BOOST_UNORDERED_DETAIL_HASH_TABLE_HPP_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/config.hpp>

#include <cstddef>
#include <cmath>
#include <algorithm>
#include <utility>

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/unordered/detail/allocator.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/and.hpp>

#include <boost/mpl/aux_/config/eti.hpp>

#if !defined(BOOST_MSVC) || BOOST_MSVC > 1200
#include <boost/compressed_pair.hpp>
#endif

// See hash_table::swap() for details about this.
#if !defined(BOOST_UNORDERED_SWAP_METHOD)
#define BOOST_UNORDERED_SWAP_METHOD 3
#endif

#if BOOST_UNORDERED_SWAP_METHOD == 1
#include <stdexcept>
#endif

namespace boost {
    namespace unordered_detail {
        template <class T> struct type_wrapper {};

        const static std::size_t default_initial_bucket_count = 50;
        inline std::size_t next_prime(std::size_t n);

        // I bet this is already in boost somewhere.

        template <class T>
        void hash_swap(T& x, T& y)
        {
            using namespace std;
            swap(x, y);
        }

        // prime number list, accessor

        static const std::size_t prime_list[] = {
            53ul, 97ul, 193ul, 389ul, 769ul,
            1543ul, 3079ul, 6151ul, 12289ul, 24593ul,
            49157ul, 98317ul, 196613ul, 393241ul, 786433ul,
            1572869ul, 3145739ul, 6291469ul, 12582917ul, 25165843ul,
            50331653ul, 100663319ul, 201326611ul, 402653189ul, 805306457ul,
            1610612741ul, 3221225473ul, 4294967291ul };

        // no throw
        inline std::size_t next_prime(std::size_t n) {
            std::size_t const* bound =
                std::lower_bound(prime_list,prime_list + 28, n);
            if(bound == prime_list + 28)
                bound--;
            return *bound;
        }

        // pair_cast - used to convert between pair types.

        template <class Dst1, class Dst2, class Src1, class Src2>
        inline std::pair<Dst1, Dst2> pair_cast(std::pair<Src1, Src2> const& x)
        {
            return std::pair<Dst1, Dst2>(Dst1(x.first), Dst2(x.second));
        }

        // Hash Table Data
        //
        // Responsible for managing the hash buckets. Has no knowledge of hash
        // functions or uniqueness.

        template <class Alloc>
        class hash_table_data
        {
        public:
            class node;
            class bucket;

            typedef std::size_t size_type;

            typedef Alloc value_allocator;

            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, node>::type
                node_allocator;
            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, bucket>::type
                bucket_allocator;

            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;
            typedef BOOST_DEDUCED_TYPENAME allocator_pointer<node_allocator>::type node_ptr;
            typedef BOOST_DEDUCED_TYPENAME allocator_pointer<bucket_allocator>::type bucket_ptr;
            typedef BOOST_DEDUCED_TYPENAME allocator_reference<value_allocator>::type reference;

#if defined(BOOST_UNORDERED_PARANOID)
            // If the allocator has the expected pointer types I take some liberties.
            BOOST_STATIC_CONSTANT(bool, is_pointer_allocator = 
                (boost::mpl::and_<
                    boost::is_same<node_ptr, node*>,
                    boost::is_same<bucket_ptr, bucket*>
                >::value));

            typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_c<
                is_pointer_allocator, bucket_ptr, node_ptr>::type link_ptr;
#else
            typedef bucket_ptr link_ptr;
#endif

            // Hash Bucket
            //
            // all no throw (memory management is performed by hash_table_data).

            class bucket
            {
                bucket& operator=(bucket const&);
            public:
                link_ptr next_;

                bucket() : next_()
                {
                }

                bucket(bucket const& x) : next_()
                {
                    // Only copy construct when allocating.
                    BOOST_ASSERT(!x.next_);
                }

                bool empty() const
                {
                    return !this->next_;
                }
            };

            // Hash Node
            //
            // all no throw

            class node : public bucket
            {
            public:
                node(value_type const& v) : bucket(), value_(v) {}

                value_type value_;
            };

#if !defined(BOOST_UNORDERED_PARANOID)
            class node_constructor
            {
                node_allocator& node_alloc_;
                bucket_allocator& bucket_alloc_;
                value_allocator value_alloc_;

                node_ptr ptr_;
                bool value_allocated_;
                bool bucket_allocated_;

            public:

                node_constructor(node_allocator& n, bucket_allocator& b)
                    : node_alloc_(n), bucket_alloc_(b), value_alloc_(n),
                    ptr_(), value_allocated_(false), bucket_allocated_(false)
                {
                }

                ~node_constructor()
                {
                    if (ptr_) {
                        if (value_allocated_)
                            value_alloc_.destroy(
                                value_alloc_.address(ptr_->value_));
                        if (bucket_allocated_)
                            bucket_alloc_.destroy(
                                bucket_alloc_.address(*ptr_));

                        node_alloc_.deallocate(ptr_, 1);
                    }
                }

                template <class V>
                void construct(V const& v)
                {
                    assert(!ptr_);
                    value_allocated_ = bucket_allocated_ = false;

                    ptr_ = node_alloc_.allocate(1);

                    bucket_alloc_.construct(bucket_alloc_.address(
                            *ptr_), bucket());
                    bucket_allocated_ = true;

                    value_alloc_.construct(value_alloc_.address(
                            ptr_->value_), v);
                    value_allocated_ = true;
                }

                // no throw
                link_ptr release()
                {
                    node_ptr p = ptr_;
                    ptr_ = node_ptr();
                    return bucket_alloc_.address(*p);
                }
            };
#else
            class node_constructor
                : public allocator_constructor<node_allocator>
            {
            public:
                node_constructor(node_allocator& n, bucket_allocator&)
                    : allocator_constructor<node_allocator>(n);
            };
#endif

            class local_iterator_base
            {
            public:
                link_ptr node_pointer_;

                local_iterator_base()
                    : node_pointer_() {}

                explicit local_iterator_base(link_ptr n)
                    : node_pointer_(n) {}

                bool not_finished() const
                {
                    return node_pointer_;
                }

                bool operator==(local_iterator_base const& x) const
                {
                    return node_pointer_ == x.node_pointer_;
                }

                bool operator!=(local_iterator_base const& x) const
                {
                    return node_pointer_ != x.node_pointer_;
                }

                reference operator*() const
                {
                    BOOST_ASSERT(node_pointer_);
                    return static_cast<node&>(*node_pointer_).value_;
                }

                void increment()
                {
                    BOOST_ASSERT(node_pointer_);
                    node_pointer_ = node_pointer_->next_;
                }
            };

            // Erase Iterator
            //
            // This is an internal 'iterator' (not an STL iterator) which is
            // used to erase or move a node.
            //
            // All no throw.

            class erase_iterator
            {
                link_ptr* prev_ptr;

            public:

                explicit erase_iterator(bucket_ptr b)
                    : prev_ptr(&b->next_) {}

                void next()
                {
                    prev_ptr = &(*prev_ptr)->next_;
                }

                bool not_finished() const
                {
                    return *prev_ptr;
                }

                value_type& operator*() const
                {
                    return static_cast<node&>(**prev_ptr).value_;
                }

                link_ptr& get_node() const
                {
                    return *prev_ptr;
                }

                bool operator!=(local_iterator_base const& y)
                {
                    return *prev_ptr != y.node_pointer_;
                }
            };

            class iterator_base
            {
            public:
                bucket_ptr bucket_;
                local_iterator_base local_;

                iterator_base()
                    : bucket_(), local_() {}

                iterator_base(bucket_ptr b)
                    : bucket_(b), local_(b->next_) {}

                iterator_base(bucket_ptr b, link_ptr n)
                    : bucket_(b), local_(n) {}

                iterator_base(bucket_ptr b, local_iterator_base const& it)
                    : bucket_(b), local_(it) {}

                local_iterator_base local() const
                {
                    return local_iterator_base(local_);
                }

                bool operator==(iterator_base const& x) const
                {
                    return local_ == x.local_;
                }

                bool operator!=(iterator_base const& x) const
                {
                    return local_ != x.local_;
                }

                reference operator*() const
                {
                    return *local_;
                }

                void increment()
                {
                    BOOST_ASSERT(bucket_);
                    local_.increment();

                    while (!local_.node_pointer_) {
                        ++bucket_;
                        local_ = local_iterator_base(bucket_->next_);
                    }
                }
            };

            // Member Variables

            node_allocator node_alloc_;
            bucket_allocator bucket_alloc_;

            bucket_ptr buckets_;
            size_type bucket_count_;
            bucket_ptr cached_begin_bucket_;
            size_type size_;

            // Constructor

            hash_table_data(size_type n, node_allocator const& a)
              : node_alloc_(a), bucket_alloc_(a),
                buckets_(), bucket_count_(next_prime(n)),
                cached_begin_bucket_(), size_(0)
            {
                // The array constructor will clean up in the event of an
                // exception.
                allocator_array_constructor<bucket_allocator>
                    constructor(bucket_alloc_);
                constructor.construct(bucket(), bucket_count_ + 1);

                cached_begin_bucket_ = constructor.get() + bucket_count_;

                // Only release the buckets once everything is successfully
                // done.
                buckets_ = constructor.release();
            }

            // no throw
            ~hash_table_data()
            {
                if(buckets_) {
                    if(buckets_[bucket_count_].next_) remove_end_marker();

                    for(size_type i = 0; i < bucket_count_; ++i)
                        delete_nodes(erase_iterator(buckets_ + i));

                    for(size_type i2 = 0; i2 < bucket_count_ + 1; ++i2)
                        bucket_alloc_.destroy(buckets_ + i2);

                    bucket_alloc_.deallocate(buckets_, bucket_count_ + 1);
                }
            }

            void add_end_marker()
            {
                BOOST_ASSERT(buckets_ && !buckets_[bucket_count_].next_);
#if !defined(BOOST_UNORDERED_PARANOID)
                buckets_[bucket_count_].next_ = buckets_ + bucket_count_;
#else
                if(is_pointer_allocator) {
                    buckets_[bucket_count_].next_ = buckets_ + bucket_count_;
                }
                else {
                    // This seems very wasteful, but I can't think of a better
                    // way to create an end node and work with all allocators.
                    // Although, I might change it to do something different
                    // when typename node_allocator::pointer == node*.
                    buckets_[bucket_count_].next_ = node_alloc_.allocate(1);
                }
#endif
            }

            void move_end_marker(hash_table_data& src)
            {
                BOOST_ASSERT(buckets_ && !buckets_[bucket_count_].next_);
                BOOST_ASSERT(src.buckets_ && src.buckets_[src.bucket_count_].next_);

#if !defined(BOOST_UNORDERED_PARANOID)
                buckets_[bucket_count_].next_ = buckets_ + bucket_count_;
#else
                if(is_pointer_allocator) {
                    buckets_[bucket_count_].next_ = buckets_ + bucket_count_;
                }
                else {
                    buckets_[bucket_count_].next_
                        = src.buckets_[src.bucket_count_].next_;
                }
#endif

                src.buckets_[src.bucket_count_].next_ = link_ptr();
            }

            void remove_end_marker()
            {
                BOOST_ASSERT(buckets_ && buckets_[bucket_count_].next_);

#if defined(BOOST_UNORDERED_PARANOID)
                if(!is_pointer_allocator)
                    node_alloc_.deallocate(static_cast<node_ptr>(
                                buckets_[bucket_count_].next_), 1);
#endif

                buckets_[bucket_count_].next_ = link_ptr();
            }

        private:

            hash_table_data(hash_table_data const&);
            hash_table_data& operator=(hash_table_data const&);

        public:

            // no throw
            void swap(hash_table_data& other)
            {
                std::swap(buckets_, other.buckets_);
                std::swap(bucket_count_, other.bucket_count_);
                std::swap(cached_begin_bucket_, other.cached_begin_bucket_);
                std::swap(size_, other.size_);
            }

            // Return the bucket index for a hashed value.
            //
            // no throw
            size_type index_from_hash(size_type hashed) const
            {
                return hashed % bucket_count_;
            }

            // Begin & End
            //
            // no throw

            iterator_base begin() const
            {
                return size_
                    ? iterator_base(cached_begin_bucket_)
                    : end();
            }

            iterator_base end() const
            {
                return iterator_base(buckets_ + bucket_count_);
            }

            local_iterator_base begin(size_type n) const
            {
                return local_iterator_base(buckets_[n].next_);
            }

            local_iterator_base end(size_type) const
            {
                return local_iterator_base();
            }

            local_iterator_base begin(bucket_ptr b) const
            {
                return local_iterator_base(b->next_);
            }

            // Bucket Size

            // no throw
            size_type bucket_size(size_type n) const
            {
                std::size_t count = 0;
                local_iterator_base it1 = begin(n);
                while(it1.not_finished()) {
                    ++count;
                    it1.increment();
                }
                return count;
            }

            // Erase iterator
            //
            // Find the pointer to a node, for use when erasing.
            //
            // no throw

            erase_iterator get_for_erase(iterator_base r) const
            {
                erase_iterator it(r.bucket_);
                local_iterator_base pos(r.local());
                while(it != pos) it.next();
                return it;
            }

            // Link/Unlink/Move Node
            //
            // For adding nodes to buckets, removing them and moving them to a
            // new bucket.
            //
            // no throw

            void link_node(link_ptr node, local_iterator_base pos)
            {
                node->next_ = pos.node_pointer_->next_;
                pos.node_pointer_->next_ = node;
                ++size_;
            }

            void link_node(link_ptr node, bucket_ptr base)
            {
                node->next_ = base->next_;
                base->next_ = node;
                ++size_;
                if(base < cached_begin_bucket_) cached_begin_bucket_ = base;
            }

            void unlink_node(erase_iterator it)
            {
                it.get_node() = it.get_node()->next_;
                --size_;
            }

            void move_node(hash_table_data& src, erase_iterator it,
                    bucket_ptr dst)
            {
                link_ptr n = it.get_node();
                src.unlink_node(it);
                link_node(n, dst);
            }

            // throws, strong exception-safety:
            link_ptr construct_node(value_type const& v)
            {
                node_constructor a(node_alloc_, bucket_alloc_);
                a.construct(v);
                return a.release();
            }

            // Create Node
            //
            // Create a node and add it to the buckets in the given position.
            //
            // strong exception safety.

            iterator_base create_node(value_type const& v, bucket_ptr base)
            {
                // throws, strong exception-safety:
                link_ptr node = construct_node(v);

                // Rest is no throw
                link_node(node, base);
                return iterator_base(base, node);
            }

            iterator_base create_node(value_type const& v, iterator_base position)
            {
                // throws, strong exception-safety:
                link_ptr node = construct_node(v);

                // Rest is no throw
                link_node(node, position.local());
                return iterator_base(position.bucket_, node);
            }

            iterator_base create_node(value_type const& v,
                    bucket_ptr base, local_iterator_base position)
            {
                // throws, strong exception-safety:
                link_ptr node = construct_node(v);

                // Rest is no throw
                if(position.not_finished())
                    link_node(node, position);
                else
                    link_node(node, base);

                return iterator_base(base, node);
            }

            // Delete Node
            //
            // Remove a node, or a range of nodes, from a bucket, and destory
            // them.
            //
            // no throw

            void delete_node(erase_iterator it)
            {
                node_ptr n = node_alloc_.address(
                        static_cast<node&>(*it.get_node()));
                unlink_node(it);

                node_alloc_.destroy(n);
                node_alloc_.deallocate(n, 1);
            }

            void delete_nodes(erase_iterator begin, local_iterator_base end)
            {
                while(begin != end) {
                    BOOST_ASSERT(begin.not_finished());
                    delete_node(begin);
                }
            }

            void delete_nodes(erase_iterator begin)
            {
                while(begin.not_finished()) delete_node(begin);
            }

            // Clear
            //
            // Remove all the nodes.
            //
            // no throw
            //
            // TODO: If delete_nodes did throw (it shouldn't but just for
            // argument's sake), could this leave cached_begin_bucket_ pointing
            // at an empty bucket?

            void clear()
            {
                bucket_ptr end = buckets_ + bucket_count_;
                while(cached_begin_bucket_ != end) {
                    delete_nodes(erase_iterator(cached_begin_bucket_));
                    ++cached_begin_bucket_;
                }
                BOOST_ASSERT(!size_);
            }

            // Erase
            //
            // Return type of erase(const_iterator):
            // http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2005/n1753.html#130
            //
            // no throw

            iterator_base erase(iterator_base r)
            {
                BOOST_ASSERT(r != end());
                iterator_base next = r;
                next.increment();
                delete_node(get_for_erase(r));
                // r has been invalidated but its bucket is still valid
                recompute_begin_bucket(r.bucket_, next.bucket_);
                return next;
            }

            iterator_base erase(iterator_base r1, iterator_base r2)
            {
                if(r1 != r2)
                {
                    BOOST_ASSERT(r1 != end());

                    if (r1.bucket_ == r2.bucket_) {
                        delete_nodes(get_for_erase(r1), r2.local());

                        // No need to call recompute_begin_bucket because
                        // the nodes are only deleted from one bucket, which
                        // still contains r2 after the erase.
                        BOOST_ASSERT(!r1.bucket_->empty());
                    }
                    else {
                        BOOST_ASSERT(r1.bucket_ < r2.bucket_);

                        delete_nodes(get_for_erase(r1));

                        for(bucket_ptr i = r1.bucket_ + 1; i != r2.bucket_; ++i)
                            delete_nodes(erase_iterator(i));

                        delete_nodes(erase_iterator(r2.bucket_), r2.local());
        
                        // r1 has been invalidated but its bucket is still
                        // valid.
                        recompute_begin_bucket(r1.bucket_, r2.bucket_);
                    }
                }

                return r2;
            }

            // recompute_begin_bucket
            //
            // After an erase cached_begin_bucket_ might be left pointing to
            // an empty bucket, so this is called to update it
            //
            // no throw

            void recompute_begin_bucket(bucket_ptr ptr)
            {
                BOOST_ASSERT(!(ptr < cached_begin_bucket_));

                if(ptr == cached_begin_bucket_)
                {
                    if (size_ != 0) {
                        while (cached_begin_bucket_->empty())
                            ++cached_begin_bucket_;
                    } else {
                        cached_begin_bucket_ = buckets_ + bucket_count_;
                    }
                }
            }

            // This is called when a range has been erased
            //
            // no throw

            void recompute_begin_bucket(bucket_ptr i, bucket_ptr j)
            {
                BOOST_ASSERT(!(i < cached_begin_bucket_) && !(j < i));
                BOOST_ASSERT(j == buckets_ + bucket_count_ || !j->empty());

                if(i == cached_begin_bucket_ && i->empty())
                    cached_begin_bucket_ = j;
            }
        };

#if defined(BOOST_MPL_CFG_MSVC_ETI_BUG)
        template <>
        class hash_table_data<int>
        {
        public:
            typedef int size_type;
            typedef int iterator_base;
        };
#endif

        template <class ValueType, class KeyType,
            class Hash, class Pred,
            class Alloc, bool EquivalentKeys>
        class hash_table
            : public hash_table_data<Alloc>
        {
            typedef hash_table_data<Alloc> data;

            typedef typename data::node_constructor node_constructor;
            typedef typename data::link_ptr link_ptr;

        public:

            typedef BOOST_DEDUCED_TYPENAME data::value_allocator value_allocator;
            typedef BOOST_DEDUCED_TYPENAME data::node_allocator node_allocator;
            typedef BOOST_DEDUCED_TYPENAME data::bucket_ptr bucket_ptr;
            typedef BOOST_DEDUCED_TYPENAME data::erase_iterator erase_iterator;

            // Type definitions

            typedef KeyType key_type;
            typedef Hash hasher;
            typedef Pred key_equal;
            typedef ValueType value_type;
            typedef std::size_t size_type;

            // iterators

            typedef BOOST_DEDUCED_TYPENAME data::local_iterator_base local_iterator_base;
            typedef BOOST_DEDUCED_TYPENAME data::iterator_base iterator_base;

        private:

            // From the compressed functions docs:
            //
            // "Finally, a word of caution for Visual C++ 6 users: if either
            // argument is an empty type, then assigning to that member will
            // produce memory corruption, unless the empty type has a "do
            // nothing" assignment operator defined. This is due to a bug in
            // the way VC6 generates implicit assignment operators."
            //
            // Nice.
            //
            // So use std::pair for Visual C++.

            class functions
            {
#if !defined(BOOST_MSVC) || BOOST_MSVC > 1200
                boost::compressed_pair<hasher, key_equal> functions_;
#else
                std::pair<hasher, key_equal> functions_;
#endif

            public:

                functions(hasher const& h, key_equal const& k)
                    : functions_(h, k) {}

                hasher const& hash_function() const
                {
#if !defined(BOOST_MSVC) || BOOST_MSVC > 1200
                    return functions_.first();
#else
                    return functions_.first;
#endif
                }

                key_equal const& key_eq() const
                {
#if !defined(BOOST_MSVC) || BOOST_MSVC > 1200
                    return functions_.second();
#else
                    return functions_.second;
#endif
                }
            };

            // Both hasher and key_equal's copy/assign can throw so double
            // buffering is used to copy them. func_ points to the currently
            // active function objects.

            typedef functions hash_table::*functions_ptr;

            functions func1_;
            functions func2_;
            functions_ptr func_;

            float mlf_;
            size_type max_load_;

        public:

            // Constructors

            hash_table(size_type n,
                    hasher const& hf, key_equal const& eq,
                    value_allocator const& a)
                : data(n, a),         // throws, cleans itself up
                func1_(hf, eq),       // throws      "     "
                func2_(hf, eq),       // throws      "     "
                func_(&hash_table::func1_), // no throw
                mlf_(1.0f)            // no throw
            {
                this->add_end_marker();
                calculate_max_load(); // no throw
            }

            // Construct from iterators

            // initial_size
            //
            // A helper function for the copy constructor to calculate how many
            // nodes will be created if the iterator's support it. Might get it
            // totally wrong for containers with unique keys.
            //
            // no throw

            template <class I>
            std::size_t initial_size(I i, I j, size_type x,
                    boost::random_access_traversal_tag)
            {
                // max load factor isn't set yet, but when it is, it'll be 1.0.
                size_type n = j - i + 1;
                return n > x ? n : x;
            };

            template <class I>
            std::size_t initial_size(I i, I j, size_type n,
                    boost::incrementable_traversal_tag)
            {
                return n;
            };

            template <class I>
            std::size_t initial_size(I i, I j, size_type x)
            {
                BOOST_DEDUCED_TYPENAME boost::iterator_traversal<I>::type
                    iterator_traversal_tag;
                return initial_size(i, j, x, iterator_traversal_tag);
            };

            template <class I>
            hash_table(I i, I j, size_type n,
                    hasher const& hf, key_equal const& eq,
                    node_allocator const& a)
                : data(initial_size(i, j, n), a),  // throws, cleans itself up
                    func1_(hf, eq),                // throws    "      "
                    func2_(hf, eq),                // throws    "      "
                    func_(&hash_table::func1_),    // no throw
                    mlf_(1.0f)                     // no throw
            {
                this->add_end_marker();
                calculate_max_load(); // no throw

                // This can throw, but hash_table_data's destructor will clean
                // up.
                insert(i, j);
            }

            // Copy Construct

            hash_table(hash_table const& x)
                : data(x.min_buckets_for_size(x.size()), x.node_alloc_), // throws
                func1_(x.current_functions()), // throws
                func2_(x.current_functions()), // throws
                func_(&hash_table::func1_), // no throw
                mlf_(x.mlf_) // no throw
            {
                this->add_end_marker();
                calculate_max_load(); // no throw

                // This can throw, but hash_table_data's destructor will clean
                // up.
                copy_buckets(x, *this, current_functions());
            }

            // Assign
            //
            // basic exception safety, if copy_functions of reserver throws
            // the container is left in a sane, empty state. If copy_buckets
            // throws the container is left with whatever was successfully
            // copied.

            hash_table& operator=(hash_table const& x)
            {
                if(this != &x)
                {
                    // TODO: I could rewrite this to use the existing nodes.
                    this->clear();                        // no throw
                    func_ = copy_functions(x);            // throws, strong
                    mlf_ = x.mlf_;                        // no throw
                    calculate_max_load();                 // no throw
                    reserve(x.size());                    // throws
                    copy_buckets(x, *this, current_functions()); // throws
                }

                return *this;
            }

            // Swap
            //
            // Swap's behaviour when allocators aren't equal is in dispute, see
            // this paper for full details:
            //
            // http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2004/n1599.html
            //
            // It lists 3 possible behaviours:
            //
            // 1 - If the allocators aren't equal then throw an exception.
            // 2 - Reallocate the elements in the containers with the
            //     appropriate allocators - messing up exception safety in
            //     the process.
            // 3 - Swap the allocators, hoping that the allocators have a
            //     no-throw swap.
            //
            // The paper recommends #3.
            //
            // I've implemented all three, but actived #3 by default, to change
            // it '#define BOOST_UNORDERED_SWAP_METHOD n' where n is the option.
            //
            // ----------------------------------------------------------------
            //
            // Strong exception safety (might change unused function objects)
            //
            // Can throw if hash or predicate object's copy constructor throws.
            // If allocators are unequal:
            //     Method 1: always throws.
            //     Method 2: can throw if copying throws
            //          (memory allocation/hash function object)
            //     Method 3: Can throw if allocator's swap throws
            //          (TODO: This one is broken right now, double buffering?)

            void swap(hash_table& x)
            {
                // This only effects the function objects that aren't in use
                // so it is strongly exception safe, via. double buffering.
                functions_ptr new_func_this = copy_functions(x);       // throws
                functions_ptr new_func_that = x.copy_functions(*this); // throws

                if(this->node_alloc_ == x.node_alloc_) {
                    this->data::swap(x); // no throw
                }
                else {
#if BOOST_UNORDERED_SWAP_METHOD == 1
                    throw std::runtime_error(
                            "Swapping containers with different allocators.");;
#elif BOOST_UNORDERED_SWAP_METHOD == 2
                    // Create new buckets in separate hash_table_data objects
                    // which will clean up if any of this throws.
                    data new_this(x.min_buckets_for_size(x.size_),
                            this->node_alloc_);                     // throws
                    copy_buckets(x, new_this, this->*new_func_this); // throws

                    data new_that(min_buckets_for_size(this->size_),
                            x.node_alloc_);                         // throws
                    x.copy_buckets(*this, new_that, x.*new_func_that); // throws

                    // Start updating the data here, no throw from now on.
                    new_this.move_end_marker(*this);                // no throw
                    new_that.move_end_marker(x);                    // no throw
                    this->data::swap(new_this);                     // no throw
                    x.data::swap(new_that);                         // no throw
#elif BOOST_UNORDERED_SWAP_METHOD == 3
                    hash_swap(this->node_alloc_,
                            x.node_alloc_);            // no throw, or is it?
                    hash_swap(this->bucket_alloc_,
                            x.bucket_alloc_);          // no throw, or is it?
                    this->data::swap(x);               // no throw
#else
#error "Invalid swap method"
#endif
                }

                // We've made it, the rest is no throw.
                std::swap(mlf_, x.mlf_);

                func_ = new_func_this;
                x.func_ = new_func_that;

                calculate_max_load();
                x.calculate_max_load();
            }

        private:

            functions const& current_functions() const
            {
                return this->*func_;
            }

            // This copies the given function objects into the currently unused
            // function objects and returns a pointer, that func_ can later be
            // set to, to commit the change.
            //
            // Strong exception safety (since only usued function objects are
            // changed).
            functions_ptr copy_functions(hash_table const& x)
            {
                // no throw:
                functions_ptr ptr = func_ == &hash_table::func1_
                    ? &hash_table::func2_ : &hash_table::func1_;
                // throws, functions not in use, so strong
                this->*ptr = x.current_functions();
                return ptr;
            }

        public:

            // accessors

            // no throw
            node_allocator const& get_allocator() const
            {
                return this->node_alloc_;
            }

            // no throw
            hasher const& hash_function() const
            {
                return current_functions().hash_function();
            }

            // no throw
            key_equal const& key_eq() const
            {
                return current_functions().key_eq();
            }

            // no throw
            size_type size() const
            {
                return this->size_;
            }

            // no throw
            bool empty() const
            {
                return this->size_ == 0;
            }

            // no throw
            size_type max_size() const
            {
                return this->node_alloc_.max_size();
            }

            // strong safety
            size_type bucket(key_type const& k) const
            {
                // hash_function can throw:
                return this->index_from_hash(hash_function()(k));
            }

            // strong safety
            bucket_ptr get_bucket(key_type const& k) const
            {
                return this->buckets_ + bucket(k);
            }

            // no throw
            size_type bucket_count() const
            {
                return this->bucket_count_;
            }

            // no throw
            size_type max_bucket_count() const
            {
                return this->bucket_alloc_.max_size();
            }

        private:

            // no throw
            size_type min_buckets_for_size(size_type n) const
            {
                BOOST_ASSERT(mlf_ != 0);

                using namespace std;

                // From 6.3.1/13:
                // size < mlf_ * count
                // => count > size / mlf_
                //
                // Or from rehash post-condition:
                // count > size / mlf_
                return static_cast<size_type>(floor(n / mlf_)) + 1;
            }

            // no throw
            void calculate_max_load()
            {
                using namespace std;

                // From 6.3.1/13:
                // Only resize when size >= mlf_ * count
                max_load_ = static_cast<size_type>(
                        ceil(mlf_ * this->bucket_count_));
            }

            // basic exception safety
            bool reserve(size_type n)
            {
                bool need_to_reserve = n >= max_load_;
                // throws - basic:
                if (need_to_reserve) rehash_impl(min_buckets_for_size(n)); 
                return need_to_reserve;
            }

        public:

            // no throw
            float max_load_factor() const
            {
                return mlf_;
            }

            // no throw
            void max_load_factor(float z)
            {
                mlf_ = z;
                calculate_max_load();
            }

            // no throw
            float load_factor() const
            {
                BOOST_ASSERT(this->bucket_count_ != 0);
                return static_cast<float>(this->size_)
                    / static_cast<float>(this->bucket_count_);
            }

        private:

            // key extractors

            // no throw
            static key_type const& extract_key(value_type const& v)
            {
                return extract(v, (type_wrapper<value_type>*)0);
            }

            static key_type const& extract(value_type const& v,
                    type_wrapper<key_type>*)
            {
                return v;
            }

            static key_type const& extract(value_type const& v,
                    void*)
            {
                return v.first;
            }

        public:

            // if hash function throws, basic exception safety
            // strong otherwise.
            void rehash(size_type n)
            {
                using namespace std;

                // no throw:
                size_type min_size = min_buckets_for_size(size());
                // basic/strong:
                rehash_impl(min_size > n ? min_size : n);

                BOOST_ASSERT(bucket_count() > size() / max_load_factor()
                        && bucket_count() >= n);
            }

        private:

            // if hash function throws, basic exception safety
            // strong otherwise
            void rehash_impl(size_type n)
            {
                n = next_prime(n); // no throw

                if (n == bucket_count())  // no throw
                    return;

                data new_buckets(n, this->node_alloc_); // throws, seperate
                move_buckets(*this, new_buckets);       // basic/no throw
                new_buckets.swap(*this);                // no throw
                calculate_max_load();                   // no throw
            }

            // move_buckets & copy_buckets
            //
            // Note: Because equivalent elements are already
            // adjacent to each other in the existing buckets, this
            // simple rehashing technique is sufficient to ensure
            // that they remain adjacent to each other in the new
            // buckets (but in reverse order).
            //
            // if the hash function throws, basic excpetion safety
            // no throw otherwise

            void move_buckets(data& src, data& dst)
            {
                BOOST_ASSERT(dst.size_ == 0);
                BOOST_ASSERT(src.node_alloc_ == dst.node_alloc_);

                bucket_ptr end = src.buckets_ + src.bucket_count_; // no throw

                for(; src.cached_begin_bucket_ != end;             // no throw
                        ++src.cached_begin_bucket_) {
                    erase_iterator it(src.cached_begin_bucket_);   // no throw
                    while(it.not_finished()) {
                        // This next line throws iff the hash function throws.
                        bucket_ptr dst_bucket = dst.buckets_ +
                            dst.index_from_hash(
                                hash_function()(extract_key(*it)));

                        dst.move_node(src, it, dst_bucket); // no throw
                    }
                }

                // Move the end marker from the source to destination.
                // Now destination is valid, source is not.
                dst.move_end_marker(src);
            }

            // basic excpetion safety, will leave dst partially filled.

            static void copy_buckets(data const& src, data& dst, functions const& f)
            {
                BOOST_ASSERT(dst.size_ == 0);

                // no throw:
                bucket_ptr end = src.buckets_ + src.bucket_count_;

                hasher const& hf = f.hash_function();

                // no throw:
                for(bucket_ptr i = src.cached_begin_bucket_; i != end; ++i) {
                    // no throw:
                    for(local_iterator_base it = src.begin(i);
                            it.not_finished(); it.increment()) {
                        // hash function can throw.
                        bucket_ptr dst_bucket = dst.buckets_ +
                            dst.index_from_hash(hf(extract_key(*it)));
                        // throws, strong
                        dst.create_node(*it, dst_bucket);
                    }
                }
            }

        public:

            // Insert functions
            //
            // basic exception safety, if hash function throws
            // strong otherwise.

            // if hash function throws, basic exception safety
            // strong otherwise
            value_type& operator[](key_type const& k)
            {
                BOOST_STATIC_ASSERT(!EquivalentKeys);
                BOOST_STATIC_ASSERT((
                            !boost::is_same<value_type, key_type>::value));
                typedef BOOST_DEDUCED_TYPENAME value_type::second_type mapped_type;

                bucket_ptr bucket = get_bucket(k);
                local_iterator_base node = find_iterator(bucket, k);

                if (node.not_finished())
                    return *node;
                else
                {
                    // Effects only in this block:

                    if (reserve(size() + 1))    // basic/strong
                        bucket = get_bucket(k); // throws, strong
                    return *this->create_node(  // throws, strong
                            value_type(k, mapped_type()),
                            bucket);
                }
            }

            // Insert for containers with equivalent keys

        private:

            // Insert node without checking if a resize is necessary.

            // strong exception safety.
            iterator_base unchecked_insert_equivalent(value_type const& v)
            {
                key_type const& k = extract_key(v);
                bucket_ptr bucket = get_bucket(k);
                local_iterator_base position = find_iterator(bucket, k);

                // No effects until here.
                return this->create_node(v, bucket, position); // throws, strong
            }

            // strong exception safety
            iterator_base unchecked_insert_equivalent(iterator_base const& it,
                    value_type const& v)
            {
                // Condition throws, no side effects.
                if(it != this->end() && equal(extract_key(v), *it)) {
                    return this->create_node(v, it);           // throws, strong
                }
                else {
                    return unchecked_insert_equivalent(v);     // throws, strong
                }
            }

        public:

            // if hash function throws, basic exception safety
            // strong otherwise
            iterator_base insert_equivalent(value_type const& v)
            {
                key_type const& k = extract_key(v);
                size_type hash_value = hash_function()(k);
                bucket_ptr bucket = this->buckets_
                    + this->index_from_hash(hash_value);
                local_iterator_base position = find_iterator(bucket, k);

                // Create the node before rehashing in case it throws.
                // throws, no side effects:
                node_constructor a(this->node_alloc_, this->bucket_alloc_);
                a.construct(v);

                // strong/no throw:
                if(reserve(size() + 1))     // basic/strong
                    bucket = this->buckets_ + this->index_from_hash(hash_value);

                // No throw from here.

                link_ptr node = a.release();

                // I'm relying on local_iterator_base not being invalidated by
                // the rehash here.
                if(position.not_finished())
                    link_node(node, position);
                else
                    link_node(node, bucket);

                return iterator_base(bucket, node);
            }

            // if hash function throws, basic exception safety
            // strong otherwise
            iterator_base insert_equivalent(iterator_base const& it, value_type const& v)
            {
                if (it != this->end() && equal(extract_key(v), *it)) { // throws, no side effects
                    // Create the node before rehashing in case it throws.
                    // throws, no side effects:
                    node_constructor a(this->node_alloc_, this->bucket_alloc_);
                    a.construct(v);

                    // The hash function can throw in get_bucket, but that's okay
                    // because then only basic exception safety is required.
                    bucket_ptr base = reserve(size() + 1) ?
                        get_bucket(extract_key(v)) : it.bucket_;

                    link_ptr node = a.release();
                    link_node(node, it.local());

                    return iterator_base(base, node);
                }
                else {
                    return insert_equivalent(v);                  // basic/strong
                }
            }

        public:

            // Insert for containers with unique keys

            // if hash function throws, basic exception safety
            // strong otherwise
            std::pair<iterator_base, bool> insert_unique(value_type const& v)
            {
                // Throws, but no side effects in this initial code
                key_type const& k = extract_key(v);
                size_type hash_value = hash_function()(k);
                bucket_ptr bucket = this->buckets_
                    + this->index_from_hash(hash_value);
                local_iterator_base pos = find_iterator(bucket, k);
                
                if (pos.not_finished()) {                       // no throw
                    // Found existing key, return it.
                    return std::pair<iterator_base, bool>(
                        iterator_base(bucket, pos), false);           // no throw

                } else {
                    // Doesn't already exist, add to bucket.
                    // Data is only changed in this block.

                    // Create the node before rehashing in case it throws.
                    // throws, no side effects:
                    node_constructor a(this->node_alloc_, this->bucket_alloc_);
                    a.construct(v);

                    // If we resize, then need to recalculate bucket.
                    if(reserve(size() + 1))                            // throws, basic/strong
                        bucket = this->buckets_ + this->index_from_hash(hash_value);

                    link_ptr node = a.release();
                    link_node(node, bucket);

                    return std::pair<iterator_base, bool>(
                        iterator_base(bucket, node), true);           // throws, strong
                }
            }

            // if hash function throws, basic exception safety
            // strong otherwise
            iterator_base insert_unique(iterator_base const& it, value_type const& v)
            {
                // If we are given an iterator pointer to the given key,
                // then just return it.
                if(it != this->end() && equal(extract_key(v), *it))  // throws, strong
                    return it;
                else
                    return insert_unique(v).first;                   // basic, if hash
            }

        private:

            // if hash function throws, basic exception safety
            // strong otherwise
            void insert(value_type const& v)
            {
                if(EquivalentKeys)
                    insert_equivalent(v);
                else
                    insert_unique(v);
            }

            // if hash function throws, basic exception safety
            // strong otherwise
            void unchecked_insert(value_type const& v)
            {
                if(EquivalentKeys)
                    unchecked_insert_equivalent(v);
                else
                    insert_unique(v);
            }

            // Insert from iterators

        private:

            // basic exception safety
            template <class I>
            void insert_for_range(I i, I j,
                    boost::random_access_traversal_tag)
            {
                reserve(size() + (j - i));                // basic/strong
                for (; i != j; ++i) unchecked_insert(*i); // strong
            }

            // basic exception safety
            template <class I>
            void insert_for_range(I i, I j,
                    boost::incrementable_traversal_tag)
            {
                for (; i != j; ++i) insert(*i); // basic/strong
            }

        public:

            // basic exception safety
            template <class InputIterator>
            void insert(InputIterator i, InputIterator j)
            {
                BOOST_DEDUCED_TYPENAME boost::iterator_traversal<InputIterator>::type
                    iterator_traversal_tag;
                insert_for_range(i, j, iterator_traversal_tag);
            }

        public:

            // erase

            // no throw
            iterator_base erase(iterator_base const& r)
            {
                return this->data::erase(r);
            }

            // strong exception safety
            size_type erase(key_type const& k)
            {
                // No side effects in initial section
                bucket_ptr bucket = get_bucket(k);
                size_type count = 0;
                erase_iterator it(find_for_erase(bucket, k));

                // Rest is no throw, side effects only after this point.
                if (it.not_finished()) {
                    if (EquivalentKeys) {
                        do {
                            ++count;
                            this->delete_node(it);
                        } while(it.not_finished() && equal(k, *it));
                    }
                    else {
                        count = 1;
                        this->delete_node(it);
                    }

                    this->recompute_begin_bucket(bucket);
                }

                return count;
            }

            // no throw
            iterator_base erase(iterator_base const& r1, iterator_base const& r2)
            {
                return this->data::erase(r1, r2);
            }

            // count
            //
            // strong exception safety, no side effects
            size_type count(key_type const& k) const
            {
                local_iterator_base it = find_iterator(k); // throws, strong
                size_type count = 0;

                if(it.not_finished()) {
                    if(EquivalentKeys) {
                        do {
                            ++count;
                            it.increment();
                        } while (it.not_finished() && equal(k, *it)); // throws, strong
                    }
                    else {
                        count = 1;
                    }
                }

                return count;
            }

            // find
            //
            // strong exception safety, no side effects
            iterator_base find(key_type const& k) const
            {
                bucket_ptr bucket = get_bucket(k);
                local_iterator_base it = find_iterator(bucket, k);

                if (it.not_finished())
                    return iterator_base(bucket, it);
                else
                    return this->end();
            }

            // equal_range
            //
            // strong exception safety, no side effects
            std::pair<iterator_base, iterator_base> equal_range(key_type const& k) const
            {
                bucket_ptr bucket = get_bucket(k);
                local_iterator_base it = find_iterator(bucket, k);
                if (it.not_finished()) {
                    local_iterator_base last = it;

                    if(EquivalentKeys) {
                        local_iterator_base next = last;
                        next.increment();

                        while(next.not_finished() && equal(k, *next)) {
                            last = next;
                            next.increment();
                        }
                    }

                    iterator_base first(iterator_base(bucket, it));
                    iterator_base second(iterator_base(bucket, last));
                    second.increment();
                    return std::pair<iterator_base, iterator_base>(first, second);
                }
                else {
                    return std::pair<iterator_base, iterator_base>(
                            this->end(), this->end());
                }
            }

        private:

            // strong exception safety, no side effects
            bool equal(key_type const& k, value_type const& v) const
            {
                return key_eq()(k, extract_key(v));
            }

            // strong exception safety, no side effects
            local_iterator_base find_iterator(key_type const& k) const
            {
                return find_iterator(get_bucket(k), k);
            }

            // strong exception safety, no side effects
            local_iterator_base find_iterator(bucket_ptr bucket,
                    key_type const& k) const
            {
                local_iterator_base it = this->begin(bucket);
                while (it.not_finished() && !equal(k, *it))
                    it.increment();

                return it;
            }

            // strong exception safety, no side effects
            erase_iterator find_for_erase(bucket_ptr bucket, key_type const& k)
                const
            {
                erase_iterator it(bucket);
                while(it.not_finished() && !equal(k, *it))
                    it.next();

                return it;
            }
        };

        // Iterators
        
        template <class Alloc> class hash_iterator;
        template <class Alloc> class hash_const_iterator;
        template <class Alloc> class hash_local_iterator;
        template <class Alloc> class hash_const_local_iterator;
        class iterator_access;

        // Local Iterators
        //
        // all no throw

        template <class Alloc>
        class hash_local_iterator
            : public boost::iterator <
                std::forward_iterator_tag,
                BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type,
                std::ptrdiff_t,
                BOOST_DEDUCED_TYPENAME allocator_pointer<Alloc>::type,
                BOOST_DEDUCED_TYPENAME allocator_reference<Alloc>::type >
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;

        private:
            typedef BOOST_DEDUCED_TYPENAME hash_table_data<Alloc>::local_iterator_base base;
            typedef hash_const_local_iterator<Alloc> const_local_iterator;

            friend class hash_const_local_iterator<Alloc>;
            base base_;

        public:
            hash_local_iterator() : base_() {}
            explicit hash_local_iterator(base x) : base_(x) {}
            BOOST_DEDUCED_TYPENAME allocator_reference<Alloc>::type operator*() const
                { return *base_; }
            value_type* operator->() const { return &*base_; }
            hash_local_iterator& operator++() { base_.increment(); return *this; }
            hash_local_iterator operator++(int) { hash_local_iterator tmp(base_); base_.increment(); return tmp; }
            bool operator==(hash_local_iterator x) const { return base_ == x.base_; }
            bool operator==(const_local_iterator x) const { return base_ == x.base_; }
            bool operator!=(hash_local_iterator x) const { return base_ != x.base_; }
            bool operator!=(const_local_iterator x) const { return base_ != x.base_; }
        };

        template <class Alloc>
        class hash_const_local_iterator
            : public boost::iterator <
                std::forward_iterator_tag,
                BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type,
                std::ptrdiff_t,
                BOOST_DEDUCED_TYPENAME allocator_const_pointer<Alloc>::type,
                BOOST_DEDUCED_TYPENAME allocator_const_reference<Alloc>::type >
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;

        private:
            typedef BOOST_DEDUCED_TYPENAME hash_table_data<Alloc>::local_iterator_base base;
            typedef hash_local_iterator<Alloc> local_iterator;
            friend class hash_local_iterator<Alloc>;
            base base_;

        public:
            hash_const_local_iterator() : base_() {}
            explicit hash_const_local_iterator(base x) : base_(x) {}
            hash_const_local_iterator(local_iterator x) : base_(x.base_) {}
            BOOST_DEDUCED_TYPENAME allocator_const_reference<Alloc>::type
                operator*() const { return *base_; }
            value_type const* operator->() const { return &*base_; }
            hash_const_local_iterator& operator++() { base_.increment(); return *this; }
            hash_const_local_iterator operator++(int) { hash_const_local_iterator tmp(base_); base_.increment(); return tmp; }
            bool operator==(local_iterator x) const { return base_ == x.base_; }
            bool operator==(hash_const_local_iterator x) const { return base_ == x.base_; }
            bool operator!=(local_iterator x) const { return base_ != x.base_; }
            bool operator!=(hash_const_local_iterator x) const { return base_ != x.base_; }
        };

        // iterators
        //
        // all no throw


        template <class Alloc>
        class hash_iterator
            : public boost::iterator <
                std::forward_iterator_tag,
                BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type,
                std::ptrdiff_t,
                BOOST_DEDUCED_TYPENAME allocator_pointer<Alloc>::type,
                BOOST_DEDUCED_TYPENAME allocator_reference<Alloc>::type >
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;

        private:
            typedef BOOST_DEDUCED_TYPENAME hash_table_data<Alloc>::iterator_base base;
            typedef hash_const_iterator<Alloc> const_iterator;
            friend class hash_const_iterator<Alloc>;
            base base_;

        public:

            hash_iterator() : base_() {}
            explicit hash_iterator(base const& x) : base_(x) {}
            BOOST_DEDUCED_TYPENAME allocator_reference<Alloc>::type
                operator*() const { return *base_; }
            value_type* operator->() const { return &*base_; }
            hash_iterator& operator++() { base_.increment(); return *this; }
            hash_iterator operator++(int) { hash_iterator tmp(base_); base_.increment(); return tmp; }
            bool operator==(hash_iterator const& x) const { return base_ == x.base_; }
            bool operator==(const_iterator const& x) const { return base_ == x.base_; }
            bool operator!=(hash_iterator const& x) const { return base_ != x.base_; }
            bool operator!=(const_iterator const& x) const { return base_ != x.base_; }
        };

        template <class Alloc>
        class hash_const_iterator
            : public boost::iterator <
                std::forward_iterator_tag,
                BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type,
                std::ptrdiff_t,
                BOOST_DEDUCED_TYPENAME allocator_const_pointer<Alloc>::type,
                BOOST_DEDUCED_TYPENAME allocator_const_reference<Alloc>::type >
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;

        private:
            typedef BOOST_DEDUCED_TYPENAME hash_table_data<Alloc>::iterator_base base;
            typedef hash_iterator<Alloc> iterator;
            friend class hash_iterator<Alloc>;
            friend class iterator_access;
            base base_;

        public:

            hash_const_iterator() : base_() {}
            explicit hash_const_iterator(base const& x) : base_(x) {}
            hash_const_iterator(iterator const& x) : base_(x.base_) {}
            BOOST_DEDUCED_TYPENAME allocator_const_reference<Alloc>::type
                operator*() const { return *base_; }
            value_type const* operator->() const { return &*base_; }
            hash_const_iterator& operator++() { base_.increment(); return *this; }
            hash_const_iterator operator++(int) { hash_const_iterator tmp(base_); base_.increment(); return tmp; }
            bool operator==(iterator const& x) const { return base_ == x.base_; }
            bool operator==(hash_const_iterator const& x) const { return base_ == x.base_; }
            bool operator!=(iterator const& x) const { return base_ != x.base_; }
            bool operator!=(hash_const_iterator const& x) const { return base_ != x.base_; }
        };

        class iterator_access
        {
        public:
            template <class Iterator>
            static BOOST_DEDUCED_TYPENAME Iterator::base const& get(Iterator const& it) {
                return it.base_;
            }
        };

        template <class ValueType, class KeyType,
            class Hash, class Pred,
            class Alloc, bool EquivalentKeys>
        class hash_types
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, ValueType>::type
                value_allocator;

            typedef hash_table<ValueType, KeyType, Hash, Pred, value_allocator,
                    EquivalentKeys> hash_table;
            typedef hash_table_data<value_allocator> data;
            typedef BOOST_DEDUCED_TYPENAME data::iterator_base iterator_base;

            typedef hash_const_local_iterator<value_allocator> const_local_iterator;
            typedef hash_local_iterator<value_allocator> local_iterator;
            typedef hash_const_iterator<value_allocator> const_iterator;
            typedef hash_iterator<value_allocator> iterator;

            typedef BOOST_DEDUCED_TYPENAME data::size_type size_type;
            typedef std::ptrdiff_t difference_type;
        };
    } // namespace boost::unordered_detail
} // namespace boost

#endif // BOOST_UNORDERED_DETAIL_HASH_TABLE_HPP_INCLUDED

