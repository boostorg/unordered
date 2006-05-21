
// Copyright (C) 2003-2004 Jeremy B. Maitin-Shepard.
// Copyright (C) 2005-2006 Daniel James

// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy
// at http://www.boost.org/LICENSE_1_0.txt)

#if BOOST_UNORDERED_HASH_EQUIVALENT
#define HASH_TABLE hash_table_equivalent_keys
#define HASH_TABLE_DATA hash_table_data_equivalent_keys
#define HASH_ITERATOR hash_iterator_equivalent_keys
#define HASH_CONST_ITERATOR hash_const_iterator_equivalent_keys
#define HASH_LOCAL_ITERATOR hash_local_iterator_equivalent_keys
#define HASH_CONST_LOCAL_ITERATOR hash_const_local_iterator_equivalent_keys
#else
#define HASH_TABLE hash_table_unique_keys
#define HASH_TABLE_DATA hash_table_data_unique_keys
#define HASH_ITERATOR hash_iterator_unique_keys
#define HASH_CONST_ITERATOR hash_const_iterator_unique_keys
#define HASH_LOCAL_ITERATOR hash_local_iterator_unique_keys
#define HASH_CONST_LOCAL_ITERATOR hash_const_local_iterator_unique_keys
#endif

namespace boost {
    namespace unordered_detail {

        //
        // Hash Table Data
        //
        // Responsible for managing the hash buckets.

        template <class Alloc>
        class HASH_TABLE_DATA
        {
        public:
            class node;
            class node_base;
            class bucket;
            typedef std::size_t size_type;

            typedef Alloc value_allocator;

            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, node>::type
                node_allocator;
            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, node_base>::type
                node_base_allocator;
            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, bucket>::type
                bucket_allocator;

            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;
            typedef BOOST_DEDUCED_TYPENAME allocator_pointer<node_allocator>::type node_ptr;
            typedef BOOST_DEDUCED_TYPENAME allocator_pointer<bucket_allocator>::type bucket_ptr;
            typedef BOOST_DEDUCED_TYPENAME allocator_reference<value_allocator>::type reference;

#if defined(BOOST_UNORDERED_PARANOID)
            // If the allocator has the expected pointer types I take some liberties.
            typedef typename boost::mpl::and_<
                    boost::is_same<node_ptr, node*>,
                    boost::is_same<bucket_ptr, bucket*>
                >::type is_pointer_allocator;

            typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                is_pointer_allocator, bucket_ptr, node_ptr>::type link_ptr;
#else
            typedef bucket_ptr link_ptr;
#endif

            // Hash Bucket
            //
            // all no throw (memory management is performed by HASH_TABLE_DATA).

            class bucket
            {
                bucket& operator=(bucket const&);
            public:
                link_ptr next_;

                bucket() : next_()
                {
                    BOOST_HASH_MSVC_RESET_PTR(next_);
                }

                bucket(bucket const& x) : next_(x.next_)
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

            class node_base : public bucket
            {
#if BOOST_UNORDERED_HASH_EQUIVALENT
            public:
                node_base() : group_next_()
                {
                    BOOST_HASH_MSVC_RESET_PTR(group_next_);
                }

                link_ptr group_next_;
#endif
            };

            class node : public node_base
            {
            public:
                node(value_type const& v) : node_base(), value_(v) {}

                value_type value_;
            };

#if !defined(BOOST_UNORDERED_PARANOID)
            class node_constructor
            {
                node_allocator& node_alloc_;
                bucket_allocator& bucket_alloc_;
                node_base_allocator node_base_alloc_;
                value_allocator value_alloc_;

                node_ptr ptr_;
                bool value_allocated_;
                bool node_base_allocated_;

            public:

                node_constructor(node_allocator& n, bucket_allocator& b)
                    : node_alloc_(n), bucket_alloc_(b), node_base_alloc_(n), value_alloc_(n),
                    ptr_(), value_allocated_(false), node_base_allocated_(false)
                {
                    BOOST_HASH_MSVC_RESET_PTR(ptr_);
                }

                ~node_constructor()
                {
                    if (ptr_) {
                        if (value_allocated_)
                            value_alloc_.destroy(
                                value_alloc_.address(ptr_->value_));
                        if (node_base_allocated_)
                            node_base_alloc_.destroy(
                                node_base_alloc_.address(*ptr_));

                        node_alloc_.deallocate(ptr_, 1);
                    }
                }

                template <class V>
                void construct(V const& v)
                {
                    BOOST_ASSERT(!ptr_);
                    value_allocated_ = node_base_allocated_ = false;

                    ptr_ = node_alloc_.allocate(1);

                    node_base_alloc_.construct(node_base_alloc_.address(
                            *ptr_), node_base());
                    node_base_allocated_ = true;

                    value_alloc_.construct(value_alloc_.address(
                            ptr_->value_), v);
                    value_allocated_ = true;
                }

                // no throw
                link_ptr release()
                {
                    node_ptr p = ptr_;
                    unordered_detail::reset(ptr_);
                    return bucket_alloc_.address(*p);
                }

            private:
                node_constructor(node_constructor const&);
                node_constructor& operator=(node_constructor const&);
            };
#else
            class node_constructor
                : public allocator_constructor<node_allocator>
            {
            public:
                node_constructor(node_allocator& n, bucket_allocator&)
                    : allocator_constructor<node_allocator>(n) {}
            };
#endif

#if BOOST_UNORDERED_HASH_EQUIVALENT
            static link_ptr& next_in_group(link_ptr p) {
                return static_cast<node&>(*p).group_next_;
            }

            // pre: Must be pointing to the first node in a group.
            static link_ptr last_in_group(link_ptr p) {
                BOOST_ASSERT(p && p != next_in_group(p)->next_);
                return next_in_group(p);
            }

            // pre: Must be pointing to the first node in a group.
            static link_ptr& next_group(link_ptr p) {
                BOOST_ASSERT(p && p != next_in_group(p)->next_);
                return next_in_group(p)->next_;
            }
#else
            static link_ptr last_in_group(link_ptr p) {
                return p;
            }

            static link_ptr& next_group(link_ptr p) {
                BOOST_ASSERT(p);
                return p->next_;
            }
#endif

            // pre: Must be pointing to a node
            static node& get_node(link_ptr p) {
                BOOST_ASSERT(p);
                return static_cast<node&>(*p);
            }

            // pre: Must be pointing to a node
            static reference get_value(link_ptr p) {
                BOOST_ASSERT(p);
                return static_cast<node&>(*p).value_;
            }

            class local_iterator_base
            {
            public:
                link_ptr node_pointer_;

                local_iterator_base()
                    : node_pointer_()
                {
                    BOOST_HASH_MSVC_RESET_PTR(node_pointer_);
                }

                explicit local_iterator_base(link_ptr n)
                    : node_pointer_(n) {}

                bool not_finished() const
                {
                    return node_pointer_ ? true : false;
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
                    return get_value(node_pointer_);
                }

                void increment()
                {
                    BOOST_ASSERT(node_pointer_);
                    node_pointer_ = node_pointer_->next_;
                }

                // pre: Must be pointing to first element in group.
                void last_in_group()
                {
                    node_pointer_ = HASH_TABLE_DATA::last_in_group(node_pointer_);
                }

                // pre: Must be pointing to first element in group.
                void next_group()
                {
                    node_pointer_ = HASH_TABLE_DATA::next_group(node_pointer_);
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

            HASH_TABLE_DATA(size_type n, node_allocator const& a)
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
            ~HASH_TABLE_DATA()
            {
                if(buckets_) {
                    if(buckets_[bucket_count_].next_) remove_end_marker();

                    for(size_type i = 0; i < bucket_count_; ++i)
                        delete_bucket_contents(buckets_ + i);

                    for(size_type i2 = 0; i2 < bucket_count_ + 1; ++i2)
                        bucket_alloc_.destroy(buckets_ + i2);

                    bucket_alloc_.deallocate(buckets_, bucket_count_ + 1);
                }
            }

            struct normal_end_marker_impl
            {
                static void add(HASH_TABLE_DATA* data) {
                    data->buckets_[data->bucket_count_].next_
                        = data->buckets_ + data->bucket_count_;
                }

                static void remove(HASH_TABLE_DATA* data) {
                    reset(data->buckets_[data->bucket_count_].next_);
                }

                static void move(HASH_TABLE_DATA* src, HASH_TABLE_DATA* dst) {
                    add(dst);
                    remove(src);
                }
            };

            struct paranoid_end_marker_impl
            {
                static void add(HASH_TABLE_DATA* data) {
                    // This seems very wasteful, but I can't think of a better
                    // way to create an end node and work with all allocators.
                    data->buckets_[data->bucket_count_].next_
                        = data->node_alloc_.allocate(1);
                }

                static void remove(HASH_TABLE_DATA* data) {
                    data->node_alloc_.deallocate(
                            data->buckets_[data->bucket_count_].next_, 1);
                }

                static void move(HASH_TABLE_DATA* src, HASH_TABLE_DATA* dst) {
                    dst->buckets_[dst->bucket_count_].next_
                        = src->buckets_[src->bucket_count_].next_;
                    reset(src->buckets_[src->bucket_count_].next_);
                }
            };

#if !defined(BOOST_UNORDERED_PARANOID)
            typedef normal_end_marker_impl end_marker_impl;
#else
            typedef typename mpl::if_<is_pointer_allocator,
                    normal_end_marker_impl,
                    paranoid_end_marker_impl>::type end_marker_impl;
#endif

            void add_end_marker()
            {
                BOOST_ASSERT(BOOST_HASH_BORLAND_BOOL(buckets_) &&
                    !buckets_[bucket_count_].next_);
                end_marker_impl::add(this);
            }

            void move_end_marker(HASH_TABLE_DATA& src)
            {
                BOOST_ASSERT(BOOST_HASH_BORLAND_BOOL(buckets_) &&
                    !buckets_[bucket_count_].next_);
                BOOST_ASSERT(BOOST_HASH_BORLAND_BOOL(src.buckets_) &&
                    BOOST_HASH_BORLAND_BOOL(src.buckets_[src.bucket_count_].next_));
                end_marker_impl::move(&src, this);
                BOOST_ASSERT(!!buckets_[bucket_count_].next_);
                BOOST_ASSERT(!src.buckets_[src.bucket_count_].next_);
            }

            void remove_end_marker()
            {
                BOOST_ASSERT(BOOST_HASH_BORLAND_BOOL(buckets_) &&
                    BOOST_HASH_BORLAND_BOOL(buckets_[bucket_count_].next_));
                end_marker_impl::remove(this);
                unordered_detail::reset(buckets_[bucket_count_].next_);
            }

        private:

            HASH_TABLE_DATA(HASH_TABLE_DATA const&);
            HASH_TABLE_DATA& operator=(HASH_TABLE_DATA const&);

        public:

            // no throw
            void swap(HASH_TABLE_DATA& other)
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

#if BOOST_UNORDERED_HASH_EQUIVALENT
            std::size_t group_count(local_iterator_base pos) const
            {
                link_ptr it = pos.node_pointer_;
                link_ptr first = it;
                size_type count = 0;
                do {
                    ++count;
                    it = next_in_group(it);
                } while (it != first); // throws, strong
                return count;
            }
#else
            std::size_t group_count(local_iterator_base) const
            {
                return 1;
            }
#endif

            // get_for_erase
            //
            // Find the pointer to a node, for use when erasing.
            //
            // no throw

#if BOOST_UNORDERED_HASH_EQUIVALENT
            link_ptr* get_for_erase(iterator_base r) const
            {
                link_ptr pos = r.local().node_pointer_;

                link_ptr* it = &next_in_group(pos)->next_;
                if(*it == pos) return it;

                it = &r.bucket_->next_;
                while(*it != pos) it = &(*it)->next_;
                return it;
            }
#else
            link_ptr* get_for_erase(iterator_base r) const
            {
                link_ptr pos = r.local().node_pointer_;
                link_ptr* it = &r.bucket_->next_;
                while(*it != pos) it = &(*it)->next_;
                return it;
            }
#endif

            // Link/Unlink/Move Node
            //
            // For adding nodes to buckets, removing them and moving them to a
            // new bucket.
            //
            // no throw

#if BOOST_UNORDERED_HASH_EQUIVALENT
            void link_node(link_ptr n, local_iterator_base pos)
            {
                node& node_ref = get_node(n);
                node& pos_ref = get_node(pos.node_pointer_);
                node_ref.next_ = pos_ref.group_next_->next_;
                node_ref.group_next_ = pos_ref.group_next_;
                pos_ref.group_next_->next_ = n;
                pos_ref.group_next_ = n;
                ++size_;
            }

            void link_node(link_ptr n, bucket_ptr base)
            {
                node& node_ref = get_node(n);
                node_ref.next_ = base->next_;
                node_ref.group_next_ = n;
                base->next_ = n;
                ++size_;
                if(base < cached_begin_bucket_) cached_begin_bucket_ = base;
            }

            void link_group(link_ptr n, bucket_ptr base)
            {
                node& node_ref = get_node(n);
                node& last_ref = get_node(node_ref.group_next_);
                last_ref.next_ = base->next_;
                base->next_ = n;

                // TODO: Use group_count...
                // or take count as a parameter - we've probably already counted
                // this when we unlinked it.
                link_ptr it = n;
                do {
                    ++size_;
                    it = next_in_group(it);
                } while(it != n);

                if(base < cached_begin_bucket_) cached_begin_bucket_ = base;
            }
#else
            void link_node(link_ptr n, bucket_ptr base)
            {
                n->next_ = base->next_;
                base->next_ = n;
                ++size_;
                if(base < cached_begin_bucket_) cached_begin_bucket_ = base;
            }

            void link_group(link_ptr n, bucket_ptr base)
            {
                link_node(n, base);
            }
#endif

#if BOOST_UNORDERED_HASH_EQUIVALENT
            // TODO: Improve this:
            void unlink_node(link_ptr* pos)
            {
                node& to_delete = get_node(*pos);
                link_ptr next = to_delete.next_;

                if(to_delete.group_next_ == *pos) {
                    // The deleted node is the sole node in the group, so
                    // no need to unlink it from a goup.
                }
                else if(next && next_in_group(next) == *pos)
                {
                    next_in_group(next) = to_delete.group_next_;
                }
                else {
                    link_ptr it = to_delete.group_next_;
                    while(next_in_group(it) != *pos) {
                        it = next_in_group(it);
                    }
                    next_in_group(it) = to_delete.group_next_;
                }
                *pos = (*pos)->next_;
                --size_;
            }

            void unlink_group(link_ptr* pos)
            {
                size_ -= group_count(local_iterator_base(*pos));
                link_ptr last = last_in_group(*pos);
                *pos = last->next_;
            }
#else
            void unlink_node(link_ptr* pos)
            {
                *pos = (*pos)->next_;
                --size_;
            }

            void unlink_group(link_ptr* pos)
            {
                *pos = (*pos)->next_;
                --size_;
            }
#endif

            void move_group(HASH_TABLE_DATA& src, bucket_ptr src_bucket, bucket_ptr dst_bucket)
            {
                link_ptr n = src_bucket->next_;
                src.unlink_group(&src_bucket->next_);
                link_group(n, dst_bucket);
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
                link_ptr n = construct_node(v);

                // Rest is no throw
                link_node(n, base);
                return iterator_base(base, n);
            }

#if BOOST_UNORDERED_HASH_EQUIVALENT
            iterator_base create_node(value_type const& v, iterator_base position)
            {
                // throws, strong exception-safety:
                link_ptr n = construct_node(v);

                // Rest is no throw
                link_node(n, position.local());
                return iterator_base(position.bucket_, n);
            }

            iterator_base create_node(value_type const& v,
                    bucket_ptr base, local_iterator_base position)
            {
                // throws, strong exception-safety:
                link_ptr n = construct_node(v);

                // Rest is no throw
                if(position.not_finished())
                    link_node(n, position);
                else
                    link_node(n, base);

                return iterator_base(base, n);
            }
#endif

#if BOOST_UNORDERED_HASH_EQUIVALENT
            void copy_group(local_iterator_base it, bucket_ptr dst)
            {
                local_iterator_base end = it;
                end.next_group();
                iterator_base pos = create_node(*it, dst);
                for(it.increment(); it != end; it.increment())
                    create_node(*it, pos);
            }
#else
            void copy_group(local_iterator_base it, bucket_ptr dst)
            {
                create_node(*it, dst);
            }
#endif

            // Delete Node
            //
            // Remove a node, or a range of nodes, from a bucket, and destory
            // them.
            //
            // no throw

            void delete_node(link_ptr* pos)
            {
                // TODO: alloc_cast
                node_ptr n = node_alloc_.address(static_cast<node&>(**pos));
                unlink_node(pos);

                node_alloc_.destroy(n);
                node_alloc_.deallocate(n, 1);
            }

            // TODO: Rename this:
            void delete_bucket_contents(link_ptr* pos)
            {
                link_ptr ptr = *pos;
                unordered_detail::reset(*pos);

                while(ptr) {
                    node_ptr n = node_alloc_.address(
                            static_cast<node&>(*ptr));
                    ptr = n->next_;

                    node_alloc_.destroy(n);
                    node_alloc_.deallocate(n, 1);
                    --size_;
                }
            }

            void delete_bucket_contents(bucket_ptr ptr)
            {
                delete_bucket_contents(&ptr->next_);
            }

#if BOOST_UNORDERED_HASH_EQUIVALENT
            link_ptr split_group(link_ptr split)
            {
                link_ptr it = split;
                if(next_in_group(it)->next_ != it)
                    return link_ptr();

                do {
                    it = next_in_group(it);
                } while(next_in_group(it)->next_ == it);

                link_ptr tmp = next_in_group(it);
                next_in_group(it) = next_in_group(split);
                next_in_group(split) = tmp;

                return it;
            }

            void split_group(link_ptr split1, link_ptr split2)
            {
                link_ptr it1 = split_group(split1);
                link_ptr it2 = split_group(split2);

                if(it1 && it1 == it2) {
                    link_ptr tmp = next_in_group(it1);
                    next_in_group(it1) = next_in_group(it2);
                    next_in_group(it2) = tmp;
                }
            }
#else
            void split_group(link_ptr)
            {
            }

            void split_group(link_ptr, link_ptr)
            {
            }
#endif

            void delete_nodes(iterator_base pos)
            {
                link_ptr* it = get_for_erase(pos);
                split_group(*it);
                delete_bucket_contents(it);
            }

            void delete_nodes(iterator_base begin, local_iterator_base end)
            {
                if(end.not_finished()) {
                    link_ptr* it = get_for_erase(begin);

                    link_ptr ptr = *it;
                    split_group(*it, end.node_pointer_);
                    *it = end.node_pointer_;

                    while(ptr != end.node_pointer_) {
                        node_ptr n = node_alloc_.address(static_cast<node&>(*ptr));
                        ptr = n->next_;

                        node_alloc_.destroy(n);
                        node_alloc_.deallocate(n, 1);
                        --size_;
                    }
                }
                else {
                    delete_nodes(begin);
                }
            }

            void delete_nodes(bucket_ptr base, local_iterator_base end)
            {
                BOOST_ASSERT(end.not_finished());
                split_group(end.node_pointer_);
            
                link_ptr ptr(base->next_);
                base->next_ = end.node_pointer_;
            
                while(ptr != end.node_pointer_) {
                    node_ptr n = node_alloc_.address(static_cast<node&>(*ptr));
                    ptr = n->next_;
            
                    node_alloc_.destroy(n);
                    node_alloc_.deallocate(n, 1);
                    --size_;
                }
            }

#if BOOST_UNORDERED_HASH_EQUIVALENT
            std::size_t delete_group(link_ptr* pos)
            {
                std::size_t count = 0;
                link_ptr first = *pos;
                link_ptr end = next_in_group(first)->next_;
                unlink_group(pos);
                while(first != end) {
                    node_ptr n = node_alloc_.address(static_cast<node&>(*first));
                    first = first->next_;
                    node_alloc_.destroy(n);
                    node_alloc_.deallocate(n, 1);
                    ++count;
                }
                return count;
            }
#else
            std::size_t delete_group(link_ptr* pos)
            {
                delete_node(pos);
                return 1;
            }
#endif

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
                    delete_bucket_contents(cached_begin_bucket_);
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
                        delete_nodes(r1, r2.local());

                        // No need to call recompute_begin_bucket because
                        // the nodes are only deleted from one bucket, which
                        // still contains r2 after the erase.
                        BOOST_ASSERT(!r1.bucket_->empty());
                    }
                    else {
                        BOOST_ASSERT(r1.bucket_ < r2.bucket_);

                        delete_nodes(r1);

                        for(bucket_ptr i = r1.bucket_ + 1; i != r2.bucket_; ++i)
                            delete_bucket_contents(i);

                        if(r2 != end()) delete_nodes(r2.bucket_, r2.local());
        
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
        class HASH_TABLE_DATA<int>
        {
        public:
            typedef int size_type;
            typedef int iterator_base;
        };
#endif

        //
        // Hash Table
        //

        template <class ValueType, class KeyType,
            class Hash, class Pred,
            class Alloc>
        class HASH_TABLE
            : public HASH_TABLE_DATA<Alloc>
        {
            typedef HASH_TABLE_DATA<Alloc> data;

            typedef typename data::node_constructor node_constructor;
            typedef typename data::link_ptr link_ptr;

        public:

            typedef BOOST_DEDUCED_TYPENAME data::value_allocator value_allocator;
            typedef BOOST_DEDUCED_TYPENAME data::node_allocator node_allocator;
            typedef BOOST_DEDUCED_TYPENAME data::bucket_ptr bucket_ptr;

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

            class functions
            {
                std::pair<hasher, key_equal> functions_;

            public:

                functions(hasher const& h, key_equal const& k)
                    : functions_(h, k) {}

                hasher const& hash_function() const
                {
                    return functions_.first;
                }

                key_equal const& key_eq() const
                {
                    return functions_.second;
                }
            };

            // Both hasher and key_equal's copy/assign can throw so double
            // buffering is used to copy them. func_ points to the currently
            // active function objects.

            typedef functions HASH_TABLE::*functions_ptr;

            functions func1_;
            functions func2_;
            functions_ptr func_;

            float mlf_;
            size_type max_load_;

        public:

            // Constructors

            HASH_TABLE(size_type n,
                    hasher const& hf, key_equal const& eq,
                    value_allocator const& a)
                : data(n, a),         // throws, cleans itself up
                func1_(hf, eq),       // throws      "     "
                func2_(hf, eq),       // throws      "     "
                func_(&HASH_TABLE::func1_), // no throw
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
            std::size_t initial_size(I i, I j, size_type n,
                    boost::forward_traversal_tag)
            {
                // max load factor isn't set yet, but when it is, it'll be 1.0.
                return (std::max)(static_cast<size_type>(std::distance(i, j)) + 1, n);
            };

            template <class I>
            std::size_t initial_size(I, I, size_type n,
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
            HASH_TABLE(I i, I j, size_type n,
                    hasher const& hf, key_equal const& eq,
                    value_allocator const& a)
                : data(initial_size(i, j, n), a),  // throws, cleans itself up
                    func1_(hf, eq),                // throws    "      "
                    func2_(hf, eq),                // throws    "      "
                    func_(&HASH_TABLE::func1_),    // no throw
                    mlf_(1.0f)                     // no throw
            {
                this->add_end_marker();
                calculate_max_load(); // no throw

                // This can throw, but HASH_TABLE_DATA's destructor will clean up.
                insert(i, j);
            }

            // Copy Construct

            HASH_TABLE(HASH_TABLE const& x)
                : data(x.min_buckets_for_size(x.size()), x.node_alloc_), // throws
                func1_(x.current_functions()), // throws
                func2_(x.current_functions()), // throws
                func_(&HASH_TABLE::func1_), // no throw
                mlf_(x.mlf_) // no throw
            {
                this->add_end_marker();
                calculate_max_load(); // no throw

                // This can throw, but HASH_TABLE_DATA's destructor will clean
                // up.
                copy_buckets(x, *this, current_functions());
            }

            // Assign
            //
            // basic exception safety, if copy_functions of reserver throws
            // the container is left in a sane, empty state. If copy_buckets
            // throws the container is left with whatever was successfully
            // copied.

            HASH_TABLE& operator=(HASH_TABLE const& x)
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

            void swap(HASH_TABLE& x)
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
                    // Create new buckets in separate HASH_TABLE_DATA objects
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
            functions_ptr copy_functions(HASH_TABLE const& x)
            {
                // no throw:
                functions_ptr ptr = func_ == &HASH_TABLE::func1_
                    ? &HASH_TABLE::func2_ : &HASH_TABLE::func1_;
                // throws, functions not in use, so strong
                this->*ptr = x.current_functions();
                return ptr;
            }

        public:

            // accessors

            // TODO: This creates an unnecessary copy.
            // no throw
            value_allocator get_allocator() const
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
                // size < mlf_ * count
                return float_to_size_t(ceil(
                        max_bucket_count() * mlf_)) - 1;
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
                // -1 to account for the end marker.
                return prev_prime(this->bucket_alloc_.max_size() - 1);
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
                max_load_ = float_to_size_t(ceil(mlf_ * this->bucket_count_));
            }

            // basic exception safety
            bool reserve(size_type n)
            {
                bool need_to_reserve = n >= max_load_;
                // throws - basic:
                if (need_to_reserve) rehash_impl(min_buckets_for_size(n));
                // TODO: Deal with this special case better:
                BOOST_ASSERT(n < max_load_ || this->bucket_count_ == max_bucket_count());
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
                BOOST_ASSERT(z > 0);
                mlf_ = (std::max)(z, minimum_max_load_factor);
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

                bucket_ptr end = src.buckets_ + src.bucket_count_;

                for(; src.cached_begin_bucket_ != end;
                        ++src.cached_begin_bucket_) {
                    bucket_ptr src_bucket = src.cached_begin_bucket_;
                    while(src_bucket->next_) {
                        // This next line throws iff the hash function throws.
                        bucket_ptr dst_bucket = dst.buckets_ +
                            dst.index_from_hash(
                                hash_function()(extract_key(
                                        get_value(src_bucket->next_))));

                        dst.move_group(src, src_bucket, dst_bucket);
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
                            it.not_finished(); it.next_group()) {
                        // hash function can throw.
                        bucket_ptr dst_bucket = dst.buckets_ +
                            dst.index_from_hash(hf(extract_key(*it)));
                        // throws, strong
                        // dst.create_node(*it, dst_bucket);
                        dst.copy_group(it, dst_bucket);
                    }
                }
            }

        public:

            // Insert functions
            //
            // basic exception safety, if hash function throws
            // strong otherwise.

#if BOOST_UNORDERED_HASH_EQUIVALENT

        private:

            // Insert node without checking if a resize is necessary.
            // (equivalent key containers)

            // strong exception safety.
            iterator_base unchecked_insert(value_type const& v)
            {
                key_type const& k = extract_key(v);
                bucket_ptr bucket = get_bucket(k);
                local_iterator_base position = find_iterator(bucket, k);

                // No effects until here, this is strong.
                return this->create_node(v, bucket, position);
            }

            // strong exception safety
            iterator_base unchecked_insert(iterator_base const& it,
                    value_type const& v)
            {
                // The condition can throw, but no side effects.
                if(it != this->end() && equal(extract_key(v), *it)) {
                    // Strong exception safety:
                    return this->create_node(v, it);
                }
                else {
                    // Strong exception safety:
                    return unchecked_insert(v);
                }
            }

        public:

            // Insert (equivalent key containers)

            // if hash function throws, basic exception safety
            // strong otherwise
            iterator_base insert(value_type const& v)
            {
                key_type const& k = extract_key(v);
                size_type hash_value = hash_function()(k);
                bucket_ptr bucket = this->buckets_
                    + this->index_from_hash(hash_value);
                local_iterator_base position = find_iterator(bucket, k);

                // Create the node before rehashing in case it throws an
                // exception (need strong safety in such a case).
                node_constructor a(this->node_alloc_, this->bucket_alloc_);
                a.construct(v);

                // reserve has basic exception safety if the hash function
                // throws, strong otherwise.
                if(reserve(size() + 1))
                    bucket = this->buckets_ + this->index_from_hash(hash_value);

                // Nothing after the point can throw.

                link_ptr n = a.release();

                // I'm relying on local_iterator_base not being invalidated by
                // the rehash here.
                if(position.not_finished())
                    this->link_node(n, position);
                else
                    this->link_node(n, bucket);

                return iterator_base(bucket, n);
            }

            // Insert (equivalent key containers)

            // if hash function throws, basic exception safety
            // strong otherwise
            iterator_base insert(iterator_base const& it, value_type const& v)
            {
                // equal can throw, but with no effects
                if (it == this->end() || !equal(extract_key(v), *it)) {
                    // Use the standard insert if the iterator doesn't point
                    // to a matching key.
                    return insert(v);
                }
                else {
                    // Create the node before rehashing in case it throws an
                    // exception (need strong safety in such a case).
                    node_constructor a(this->node_alloc_, this->bucket_alloc_);
                    a.construct(v);

                    // reserve has basic exception safety if the hash function
                    // throws, strong otherwise.
                    bucket_ptr base = reserve(size() + 1) ?
                        get_bucket(extract_key(v)) : it.bucket_;

                    // Nothing after this point can throw

                    link_ptr n = a.release();
                    this->link_node(n, it.local());

                    return iterator_base(base, n);
                }
            }

            // Insert from iterator range (equivalent key containers)

        private:
            // if hash function throws, or inserting > 1 element, basic exception safety
            // strong otherwise
            template <class I>
            void insert_for_range(I i, I j, forward_traversal_tag)
            {
                std::size_t distance = std::distance(i, j);
                if(distance == 1) {
                    insert(*i);
                }
                else {
                    // Only require basic exception safety here
                    reserve(size() + distance);
                    for (; i != j; ++i) unchecked_insert(*i);
                }
            }

            // if hash function throws, or inserting > 1 element, basic exception safety
            // strong otherwise
            template <class I>
            void insert_for_range(I i, I j,
                    boost::incrementable_traversal_tag)
            {
                // If only inserting 1 element, get the required
                // safety since insert is only called once.
                for (; i != j; ++i) insert(*i);
            }

        public:

            // if hash function throws, or inserting > 1 element, basic exception safety
            // strong otherwise
            template <class InputIterator>
            void insert(InputIterator i, InputIterator j)
            {
                BOOST_DEDUCED_TYPENAME boost::iterator_traversal<InputIterator>::type
                    iterator_traversal_tag;
                insert_for_range(i, j, iterator_traversal_tag);
            }
#else
            // if hash function throws, basic exception safety
            // strong otherwise
            value_type& operator[](key_type const& k)
            {
                BOOST_STATIC_ASSERT((
                            !boost::is_same<value_type, key_type>::value));
                typedef BOOST_DEDUCED_TYPENAME value_type::second_type mapped_type;

                bucket_ptr bucket = get_bucket(k);
                local_iterator_base pos = find_iterator(bucket, k);

                if (pos.not_finished())
                    return *pos;
                else
                {
                    // Side effects only in this block.

                    // Create the node before rehashing in case it throws an
                    // exception (need strong safety in such a case).
                    node_constructor a(this->node_alloc_, this->bucket_alloc_);
                    a.construct(value_type(k, mapped_type()));

                    // reserve has basic exception safety if the hash function
                    // throws, strong otherwise.
                    if (reserve(size() + 1)) {
                        // get_bucket can only throw if the hash function throws (in
                        // which case basic exception safety is okay).
                        bucket = get_bucket(k);
                    }

                    // Nothing after this point can throw.

                    link_ptr n = a.release();
                    this->link_node(n, bucket);

                    return *local_iterator_base(n);
                }
            }

            // Insert (unique keys)

            // if hash function throws, basic exception safety
            // strong otherwise
            std::pair<iterator_base, bool> insert(value_type const& v)
            {
                // No side effects in this initial code
                key_type const& k = extract_key(v);
                size_type hash_value = hash_function()(k);
                bucket_ptr bucket = this->buckets_
                    + this->index_from_hash(hash_value);
                local_iterator_base pos = find_iterator(bucket, k);
                
                if (pos.not_finished()) {
                    // Found an existing key, return it (no throw).
                    return std::pair<iterator_base, bool>(
                        iterator_base(bucket, pos), false);

                } else {
                    // Doesn't already exist, add to bucket.
                    // Side effects only in this block.

                    // Create the node before rehashing in case it throws an
                    // exception (need strong safety in such a case).
                    node_constructor a(this->node_alloc_, this->bucket_alloc_);
                    a.construct(v);

                    // reserve has basic exception safety if the hash function
                    // throws, strong otherwise.
                    if(reserve(size() + 1))
                        bucket = this->buckets_ + this->index_from_hash(hash_value);

                    // Nothing after this point can throw.

                    link_ptr n = a.release();
                    this->link_node(n, bucket);

                    return std::pair<iterator_base, bool>(
                        iterator_base(bucket, n), true);
                }
            }

            // Insert (unique keys)

            // if hash function throws, basic exception safety
            // strong otherwise
            iterator_base insert(iterator_base const& it, value_type const& v)
            {
                if(it != this->end() && equal(extract_key(v), *it))
                    return it;
                else
                    return insert(v).first;
            }

            // Insert from iterators (unique keys)

            // if hash function throws, or inserting > 1 element, basic exception safety
            // strong otherwise
            template <class InputIterator>
            void insert(InputIterator i, InputIterator j)
            {
                // If only inserting 1 element, get the required
                // safety since insert is only called once.
                for (; i != j; ++i) insert(*i);
            }
#endif

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
                link_ptr* it = find_for_erase(bucket, k);

                // The rest is no throw.
                if (*it) {
                    size_type count = delete_group(it);
                    this->recompute_begin_bucket(bucket);
                    return count;
                }
                else {
                    return 0;
                }
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
                return it.not_finished() ? group_count(it) : 0;
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
                    last.last_in_group();

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
                    it.next_group();

                return it;
            }

            // strong exception safety, no side effects
            link_ptr* find_for_erase(bucket_ptr bucket, key_type const& k) const
            {
                link_ptr* it = &bucket->next_;
                while(*it && !equal(k, this->get_value(*it)))
                    it = &this->next_group(*it);

                return it;
            }
        };

        // Iterators
        
        template <class Alloc> class HASH_ITERATOR;
        template <class Alloc> class HASH_CONST_ITERATOR;
        template <class Alloc> class HASH_LOCAL_ITERATOR;
        template <class Alloc> class HASH_CONST_LOCAL_ITERATOR;
        class iterator_access;

        // Local Iterators
        //
        // all no throw

        template <class Alloc>
        class HASH_LOCAL_ITERATOR
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
            typedef BOOST_DEDUCED_TYPENAME HASH_TABLE_DATA<Alloc>::local_iterator_base base;
            typedef HASH_CONST_LOCAL_ITERATOR<Alloc> const_local_iterator;

            friend class HASH_CONST_LOCAL_ITERATOR<Alloc>;
            base base_;

        public:
            HASH_LOCAL_ITERATOR() : base_() {}
            explicit HASH_LOCAL_ITERATOR(base x) : base_(x) {}
            BOOST_DEDUCED_TYPENAME allocator_reference<Alloc>::type operator*() const
                { return *base_; }
            value_type* operator->() const { return &*base_; }
            HASH_LOCAL_ITERATOR& operator++() { base_.increment(); return *this; }
            HASH_LOCAL_ITERATOR operator++(int) { HASH_LOCAL_ITERATOR tmp(base_); base_.increment(); return tmp; }
            bool operator==(HASH_LOCAL_ITERATOR x) const { return base_ == x.base_; }
            bool operator==(const_local_iterator x) const { return base_ == x.base_; }
            bool operator!=(HASH_LOCAL_ITERATOR x) const { return base_ != x.base_; }
            bool operator!=(const_local_iterator x) const { return base_ != x.base_; }
        };

        template <class Alloc>
        class HASH_CONST_LOCAL_ITERATOR
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
            typedef BOOST_DEDUCED_TYPENAME HASH_TABLE_DATA<Alloc>::local_iterator_base base;
            typedef HASH_LOCAL_ITERATOR<Alloc> local_iterator;
            friend class HASH_LOCAL_ITERATOR<Alloc>;
            base base_;

        public:
            HASH_CONST_LOCAL_ITERATOR() : base_() {}
            explicit HASH_CONST_LOCAL_ITERATOR(base x) : base_(x) {}
            HASH_CONST_LOCAL_ITERATOR(local_iterator x) : base_(x.base_) {}
            BOOST_DEDUCED_TYPENAME allocator_const_reference<Alloc>::type
                operator*() const { return *base_; }
            value_type const* operator->() const { return &*base_; }
            HASH_CONST_LOCAL_ITERATOR& operator++() { base_.increment(); return *this; }
            HASH_CONST_LOCAL_ITERATOR operator++(int) { HASH_CONST_LOCAL_ITERATOR tmp(base_); base_.increment(); return tmp; }
            bool operator==(local_iterator x) const { return base_ == x.base_; }
            bool operator==(HASH_CONST_LOCAL_ITERATOR x) const { return base_ == x.base_; }
            bool operator!=(local_iterator x) const { return base_ != x.base_; }
            bool operator!=(HASH_CONST_LOCAL_ITERATOR x) const { return base_ != x.base_; }
        };

        // iterators
        //
        // all no throw


        template <class Alloc>
        class HASH_ITERATOR
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
            typedef BOOST_DEDUCED_TYPENAME HASH_TABLE_DATA<Alloc>::iterator_base base;
            typedef HASH_CONST_ITERATOR<Alloc> const_iterator;
            friend class HASH_CONST_ITERATOR<Alloc>;
            base base_;

        public:

            HASH_ITERATOR() : base_() {}
            explicit HASH_ITERATOR(base const& x) : base_(x) {}
            BOOST_DEDUCED_TYPENAME allocator_reference<Alloc>::type
                operator*() const { return *base_; }
            value_type* operator->() const { return &*base_; }
            HASH_ITERATOR& operator++() { base_.increment(); return *this; }
            HASH_ITERATOR operator++(int) { HASH_ITERATOR tmp(base_); base_.increment(); return tmp; }
            bool operator==(HASH_ITERATOR const& x) const { return base_ == x.base_; }
            bool operator==(const_iterator const& x) const { return base_ == x.base_; }
            bool operator!=(HASH_ITERATOR const& x) const { return base_ != x.base_; }
            bool operator!=(const_iterator const& x) const { return base_ != x.base_; }
        };

        template <class Alloc>
        class HASH_CONST_ITERATOR
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
            typedef BOOST_DEDUCED_TYPENAME HASH_TABLE_DATA<Alloc>::iterator_base base;
            typedef HASH_ITERATOR<Alloc> iterator;
            friend class HASH_ITERATOR<Alloc>;
            friend class iterator_access;
            base base_;

        public:

            HASH_CONST_ITERATOR() : base_() {}
            explicit HASH_CONST_ITERATOR(base const& x) : base_(x) {}
            HASH_CONST_ITERATOR(iterator const& x) : base_(x.base_) {}
            BOOST_DEDUCED_TYPENAME allocator_const_reference<Alloc>::type
                operator*() const { return *base_; }
            value_type const* operator->() const { return &*base_; }
            HASH_CONST_ITERATOR& operator++() { base_.increment(); return *this; }
            HASH_CONST_ITERATOR operator++(int) { HASH_CONST_ITERATOR tmp(base_); base_.increment(); return tmp; }
            bool operator==(iterator const& x) const { return base_ == x.base_; }
            bool operator==(HASH_CONST_ITERATOR const& x) const { return base_ == x.base_; }
            bool operator!=(iterator const& x) const { return base_ != x.base_; }
            bool operator!=(HASH_CONST_ITERATOR const& x) const { return base_ != x.base_; }
        };
    }
}

#undef HASH_TABLE
#undef HASH_TABLE_DATA
#undef HASH_ITERATOR
#undef HASH_CONST_ITERATOR
#undef HASH_LOCAL_ITERATOR
#undef HASH_CONST_LOCAL_ITERATOR
