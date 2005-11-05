
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/unordered/detail/hash_table.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_UNIT_TEST(next_prime_test)
{
    BOOST_CHECK_EQUAL(53ul, boost::unordered_detail::next_prime(0));
    BOOST_CHECK_EQUAL(53ul, boost::unordered_detail::next_prime(52));
    BOOST_CHECK_EQUAL(53ul, boost::unordered_detail::next_prime(53));
    BOOST_CHECK_EQUAL(97ul, boost::unordered_detail::next_prime(54));
    BOOST_CHECK_EQUAL(98317ul, boost::unordered_detail::next_prime(50000ul));
}
