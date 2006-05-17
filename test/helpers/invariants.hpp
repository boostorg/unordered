
//  Copyright Daniel James 2006. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// This header contains metafunctions/functions to get the equivalent
// associative container for an unordered container, and compare the contents.

#if !defined(BOOST_UNORDERED_TEST_HELPERS_INVARIANT_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_INVARIANT_HEADER

#include <set>
#include "./metafunctions.hpp"
#include "./helpers.hpp"

namespace test
{
    template <class X>
    void check_equivalent_keys(X const& x1)
    {
        typename X::key_equal eq = x1.key_eq();
        typedef typename X::key_type key_type;
        std::set<key_type> found_;

        typename X::const_iterator it = x1.begin(), end = x1.end();
        typename X::size_type size = 0;
        while(it != end) {
            // First test that the current key has not occured before, required
            // to test either that keys are unique or that equivalent keys are
            // adjacent. (6.3.1/6)
            key_type key = get_key<X>(*it);
            if(found_.find(key) != found_.end())
                BOOST_ERROR("Elements with equivalent keys aren't adjacent.");
            found_.insert(key);

            // Iterate over equivalent keys, counting them.
            unsigned int count = 0;
            do {
                ++it;
                ++count;
                ++size;
            } while(it != end && eq(get_key<X>(*it), key));

            // If the container has unique keys, test that there's only one.
            // Since the previous test makes sure that all equivalent keys are
            // adjacent, this is all the equivalent keys - so the test is
            // sufficient. (6.3.1/6 again).
            if(test::has_unique_keys<X>::value && count != 1)
                BOOST_ERROR("Non-unique key.");

            if(x1.count(key) != count)
                BOOST_ERROR("Incorrect output of count.");

            // Check that the keys are in the correct bucket and are adjacent in
            // the bucket.
            typename X::size_type bucket = x1.bucket(key);
            typename X::const_local_iterator lit = x1.begin(bucket), lend = x1.end(bucket);
            for(; lit != lend && !eq(get_key<X>(*lit), key); ++lit) continue;
            if(lit == lend)
                BOOST_ERROR("Unable to find element with a local_iterator");
            unsigned int count2 = 0;
            for(; lit != lend && eq(get_key<X>(*lit), key); ++lit) ++count2;
            if(count != count2)
                BOOST_ERROR("Element count doesn't match local_iterator.");
            for(; lit != lend; ++lit) {
                if(eq(get_key<X>(*lit), key)) {
                    BOOST_ERROR("Non-adjacent element with equivalent key in bucket.");
                    break;
                }
            }
        };

        // Finally, check that size matches up.
        if(x1.size() != size)
            BOOST_ERROR("x1.size() doesn't match actual size.");
        if(static_cast<float>(size) / x1.bucket_count() != x1.load_factor())
            BOOST_ERROR("x1.load_factor() doesn't match actual load_factor.");
    }
}

#endif

