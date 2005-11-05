
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "./helpers/unit_test.hpp"
#include "./helpers/exception_test.hpp"
#include "./helpers/random_values.hpp"
#include "./helpers/constructors.hpp"
#include "./equivalent.hpp"
#include "./invariant.hpp"

// 23.1/5 + TR1 6.3.1/9 row 12
META_FUNC_TEST_CASE(empty_copy_test1, X)
{
    X x;
    test::unordered_equivalence_tester<X> equivalent(x);

    EXCEPTION_TEST(1000)
    {
        X y(x);
        equivalent.test(y);
        test::invariant_check(y);
    }
    EXCEPTION_TEST_END

    test::invariant_check(x);
}

META_FUNC_TEST_CASE(empty_copy_test2, X)
{
    test::constructors<X> constructor;
    X x(100, constructor.hasher(55), constructor.key_equal(55), constructor.allocator(10));
    x.max_load_factor(4.0);
    test::unordered_equivalence_tester<X> equivalent(x);

    EXCEPTION_TEST(1000)
    {
        X y(x);
        equivalent.test(y);
        test::invariant_check(y);
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(range_copy_construct,X)
{
    test::constructors<X> constructor;
    test::random_values<X> values(10);

    X x(values.begin(), values.end(), 100,
        constructor.hasher(55), constructor.key_equal(55), constructor.allocator(10));
    x.max_load_factor(4.0);
    test::unordered_equivalence_tester<X> equivalent(x);

    EXCEPTION_TEST(1000)
    {
        X y(x);
        equivalent.test(y);
        test::invariant_check(y);
    }
    EXCEPTION_TEST_END
}

template <class X>
void check_container(X const& x, test::unordered_equivalence_tester<X> const& equivalent)
{
    equivalent.test(x);
    test::invariant_check(x);
}

META_FUNC_TEST_CASE(anon_copy_construct, X)
{
    test::constructors<X> constructor;
    test::random_values<X> values(10);

    X x(values.begin(), values.end(), 100,
        constructor.hasher(55), constructor.key_equal(55), constructor.allocator(10));
    x.max_load_factor(4.0);
    test::unordered_equivalence_tester<X> equivalent(x);

    EXCEPTION_TEST(1000)
    {
        check_container(X(x), equivalent);
    }
    EXCEPTION_TEST_END
}

template <class X>
X return_container(X const& x)
{
    return x;
}

META_FUNC_TEST_CASE(copy_from_return,X)
{
    test::random_values<X> values(10);
    X x(values.begin(), values.end());
    test::unordered_equivalence_tester<X> equivalent(x);

    EXCEPTION_TEST(1000)
    {
        X y(return_container(x));
        equivalent.test(y);
        test::invariant_check(y);
    }
    EXCEPTION_TEST_END
}

AUTO_META_TESTS(
    (empty_copy_test1)(empty_copy_test2)(range_copy_construct)
    (anon_copy_construct)(copy_from_return),
    CONTAINER_SEQ
)
