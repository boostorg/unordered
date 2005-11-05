
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <boost/iterator/counting_iterator.hpp>

#include <boost/limits.hpp>

// If the bucket count is higher than the upper bounds in this file, it's not a
// bug (but it's not that great either). There are also some tests on when the
// number of buckets is increased on an insert, these tests are checking that
// some implementation details are working - if the implementation is changed
// they can be removed.
//
// TODO: the argument to max_load_factor is just a hint, so test against the
// container's actual max_load_factor.

BOOST_AUTO_UNIT_TEST(test_rehash)
{
    boost::counting_iterator<int> begin(0);
    boost::counting_iterator<int> end(1000);

    boost::unordered_set<int> x1(begin, end);
    BOOST_CHECK(x1.bucket_count() >= 1000);
    BOOST_WARN(x1.bucket_count() < 2000);

    x1.max_load_factor(0.5);
    x1.rehash(0);
    BOOST_CHECK(x1.bucket_count() >= 2000);
    BOOST_WARN(x1.bucket_count() < 4000);
    x1.rehash(0);
    BOOST_CHECK(x1.bucket_count() >= 2000);
    BOOST_WARN(x1.bucket_count() < 4000);

    x1.max_load_factor(2.0);
    x1.rehash(0);
    BOOST_CHECK(x1.bucket_count() >= 500);
    BOOST_WARN(x1.bucket_count() < 1000);
    x1.rehash(1500);
    BOOST_CHECK(x1.bucket_count() >= 1500);
    BOOST_WARN(x1.bucket_count() < 3000);

    x1.max_load_factor(0.5);
    x1.rehash(0);
    BOOST_CHECK(x1.bucket_count() >= 2000);
    BOOST_WARN(x1.bucket_count() < 4000);
}

BOOST_AUTO_UNIT_TEST(test_insert_range)
{
    boost::counting_iterator<int> begin(0);
    boost::counting_iterator<int> end(1000);

    boost::unordered_set<int> x1(begin, end);
    BOOST_CHECK(x1.bucket_count() >= 1000);
    BOOST_WARN(x1.bucket_count() < 2000);

    x1.clear();
    x1.max_load_factor(0.5);
    x1.rehash(0);
    x1.insert(begin, end);
    BOOST_CHECK(x1.bucket_count() >= 2000);
    BOOST_WARN(x1.bucket_count() < 4000);

    x1.clear();
    x1.max_load_factor(2.0);
    x1.rehash(0);
    x1.insert(begin, end);
    BOOST_CHECK(x1.bucket_count() >= 500);
    BOOST_WARN(x1.bucket_count() < 1000);
}

BOOST_AUTO_UNIT_TEST(test_insert)
{
    boost::unordered_map<int, int> x1;

    size_t i;
    for(i = 0; i < 1000; ++i)
    {
        size_t old_bucket_count = x1.bucket_count();
        x1[i] = i;
        BOOST_CHECK(i <= x1.bucket_count());
        BOOST_CHECK(x1.bucket_count() == old_bucket_count ||
                x1.size() >= old_bucket_count);

        // This isn't really required:
        BOOST_WARN(x1.size() < x1.bucket_count());
    }

    x1.clear();
    x1.max_load_factor(2.0);
    x1.rehash(0);

    for(i = 0; i < 1000; ++i)
    {
        size_t old_bucket_count = x1.bucket_count();
        x1[i] = i;
        BOOST_CHECK(i <= x1.bucket_count() * 2);
        BOOST_CHECK(x1.bucket_count() == old_bucket_count ||
                x1.size() >= old_bucket_count * 2);

        // This isn't really required:
        BOOST_WARN(x1.size() < x1.bucket_count() * 2);
    }

    x1.clear();
    x1.rehash(0);
    x1.max_load_factor(0.5);

    for(i = 0; i < 1000; ++i)
    {
        size_t old_bucket_count = x1.bucket_count();
        x1[i] = i;
        BOOST_CHECK(i * 2 <= x1.bucket_count());
        BOOST_CHECK(x1.bucket_count() == old_bucket_count ||
                x1.size() * 2 >= old_bucket_count);

        // This isn't really required:
        BOOST_WARN(x1.size() * 2 < x1.bucket_count());
    }
}

BOOST_AUTO_UNIT_TEST(test_large_mlf)
{
    boost::unordered_set<int> x1;
    x1.max_load_factor(static_cast<float>(
            (std::numeric_limits<boost::unordered_set<int>::size_type>::max)()
        ) * 10);

    boost::unordered_map<int, int>::size_type bucket_count = x1.bucket_count();

    for(int i = 0; i < 1000; ++i)
    {
        x1.insert(i);
        BOOST_CHECK(x1.bucket_count() == bucket_count);
    }

    boost::counting_iterator<int> begin(1000);
    boost::counting_iterator<int> end(2000);
    x1.insert(begin, end);
    BOOST_CHECK(x1.bucket_count() == bucket_count);
}

BOOST_AUTO_UNIT_TEST(test_infinite_mlf)
{
    if(std::numeric_limits<float>::has_infinity)
    {
        boost::unordered_set<int> x1;
        x1.max_load_factor(std::numeric_limits<float>::infinity());

        boost::unordered_map<int, int>::size_type bucket_count = x1.bucket_count();

        for(int i = 0; i < 1000; ++i)
        {
            x1.insert(i);
            BOOST_CHECK(x1.bucket_count() == bucket_count);
        }

        boost::counting_iterator<int> begin(1000);
        boost::counting_iterator<int> end(2000);
        x1.insert(begin, end);
        BOOST_CHECK(x1.bucket_count() == bucket_count);
    }
}
