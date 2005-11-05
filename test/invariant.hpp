
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_CONTAINER_INVARIANT_HEADER)
#define BOOST_UNORDERED_TEST_CONTAINER_INVARIANT_HEADER

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/test/test_tools.hpp>
#include "./helpers/invariant_checker.hpp"
#include "./helpers/functional.hpp"

namespace test
{
    template <class Key, class Pred>
    bool check_matches(Key const&, Pred const&)
    {
        return true;
    }

    bool check_matches(hash const& h, equals const& p)
    {
        // TODO: This isn't actually true - change functional so that it is.
        BOOST_CHECK_EQUAL(h.offset, p.tag);
        return true;
    }

    template <class Container>
    void invariant_check_container(Container const& x)
    {
        // Check that the begin and end iterators match the container size.
        // (And also that you can iterate through all the elements).
        BOOST_CHECK_EQUAL((std::size_t) std::distance(x.begin(), x.end()),
                x.size());

        BOOST_CHECK(check_matches(x.hash_function(), x.key_eq()));

        // It is possible for this to legally fail, eg. if you
        // set max_load_factor to lower than the current load factor and
        // don't give the table a chance to rehash.
        BOOST_WARN(x.load_factor() <= x.max_load_factor());
    }

    template <class Container>
    void invariant_check_set(Container const& x)
    {
        invariant_check_container(x);

        // Check that the elements are in the correct buckets.
        std::size_t count = 0;
        std::size_t bucket_count = x.bucket_count();
        for(std::size_t i = 0; i < bucket_count; ++i) {
            std::size_t bucket_size = 0;
            for(typename Container::const_local_iterator j = x.begin(i);
                    j != x.end(i); ++j)
            {
                ++bucket_size;
                BOOST_CHECK_EQUAL(i, x.bucket(*j));
                BOOST_CHECK_EQUAL(i,
                        x.hash_function()(*j) % bucket_count);
            }
            BOOST_CHECK_EQUAL(bucket_size, x.bucket_size(i));

            count += bucket_size;
        }

        // Check that the size matches what we've just seen.
        BOOST_CHECK_EQUAL(count, x.size());
    }

    template <class Container>
    void invariant_check_map(Container const& x)
    {
        invariant_check_container(x);

        // Check that the elements are in the correct buckets.
        std::size_t count = 0;
        std::size_t bucket_count = x.bucket_count();
        for(std::size_t i = 0; i < bucket_count; ++i) {
            std::size_t bucket_size = 0;
            for(typename Container::const_local_iterator j = x.begin(i);
                    j != x.end(i); ++j)
            {
                ++bucket_size;
                BOOST_CHECK_EQUAL(i, x.bucket(j->first));
                BOOST_CHECK_EQUAL(i,
                        x.hash_function()(j->first) % bucket_count);
            }
            BOOST_CHECK_EQUAL(bucket_size, x.bucket_size(i));

            count += bucket_size;
        }

        // Check that the size matches what we've just seen.
        BOOST_CHECK_EQUAL(count, x.size());
    }
}

namespace boost
{
    template <class T, class Hash, class Pred, class Alloc>
    void invariant_impl(boost::unordered_set<T, Hash, Pred, Alloc> const& x)
    {
        test::invariant_check_set(x);
    }

    template <class T, class Hash, class Pred, class Alloc>
    void invariant_impl(boost::unordered_multiset<T, Hash, Pred, Alloc> const& x)
    {
        test::invariant_check_set(x);
    }

    template <class K, class T, class Hash, class Pred, class Alloc>
    void invariant_impl(boost::unordered_map<K, T, Hash, Pred, Alloc> const& x)
    {
        test::invariant_check_map(x);
    }

    template <class K, class T, class Hash, class Pred, class Alloc>
    void invariant_impl(boost::unordered_multimap<K, T, Hash, Pred, Alloc> const& x)
    {
        test::invariant_check_map(x);
    }
}

#endif
