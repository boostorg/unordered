
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/limits.hpp>
#include <boost/test/test_tools.hpp>
#include "./helpers/unit_test.hpp"
#include "./helpers/random_values.hpp"
#include "./equivalent.hpp"
#include "./check_return_type.hpp"

typedef double comparison_type;

// 23.1/5
template <class X, class T>
void container_tests(X*, T*)
{
    typedef typename X::iterator iterator;
    typedef typename X::const_iterator const_iterator;
    typedef typename X::difference_type difference_type;
    typedef typename X::size_type size_type;

    BOOST_MPL_ASSERT((boost::is_same<typename X::value_type, T>));
    // TODO: Actually 'lvalue of T'/'const lvalue of T'
    BOOST_MPL_ASSERT((boost::is_same<typename X::reference, T&>));
    BOOST_MPL_ASSERT((boost::is_same<typename X::const_reference, T const&>));

    // TODO: Iterator checks.
    BOOST_MPL_ASSERT((boost::is_same<typename iterator::value_type, T>));
    BOOST_MPL_ASSERT_NOT((boost::is_same<boost::BOOST_ITERATOR_CATEGORY<iterator>, std::output_iterator_tag>));
    BOOST_MPL_ASSERT((boost::is_convertible<iterator, const_iterator>));

    BOOST_MPL_ASSERT((boost::is_same<typename const_iterator::value_type, T>));
    BOOST_MPL_ASSERT_NOT((boost::is_same<boost::BOOST_ITERATOR_CATEGORY<typename X::const_iterator>, std::output_iterator_tag>));

    BOOST_MPL_ASSERT((boost::mpl::bool_<std::numeric_limits<difference_type>::is_signed>));
    BOOST_MPL_ASSERT((boost::mpl::bool_<std::numeric_limits<difference_type>::is_integer>));
    BOOST_MPL_ASSERT((boost::is_same<typename iterator::difference_type, difference_type>));
    BOOST_MPL_ASSERT((boost::is_same<typename const_iterator::difference_type, difference_type>));
    
    BOOST_MPL_ASSERT_NOT((boost::mpl::bool_<std::numeric_limits<size_type>::is_signed>));
    BOOST_MPL_ASSERT((boost::mpl::bool_<std::numeric_limits<size_type>::is_integer>));
    BOOST_CHECK((comparison_type)(std::numeric_limits<size_type>::max)()
        > (comparison_type)(std::numeric_limits<difference_type>::max)());
    
    {
        X u;
        BOOST_CHECK(u.size() == 0);
        BOOST_CHECK(X().size() == 0);
    }
}

template <class X>
void container_tests2(X& a)
{
    // 23.1/5 continued
    typedef typename X::iterator iterator;
    typedef typename X::const_iterator const_iterator;
    typedef typename X::difference_type difference_type;
    typedef typename X::size_type size_type;

    test::unordered_equivalence_tester<X> equivalent(a);

    {
        X u(a);
        equivalent.test(u);
    }

    {
        X u = a;
        equivalent.test(u);
    }

    // Test that destructor destructs all elements (already done by test::allocator/test::member?).
    
    {
        X const& a_const = a;
        
        check_return_type<iterator>::equals(a.begin());
        check_return_type<const_iterator>::equals(a_const.begin());
        check_return_type<iterator>::equals(a.end());
        check_return_type<const_iterator>::equals(a_const.end());
    }
    
    // No tests for ==, != since they're not required for unordered containers.
    
    {
        X b;
        a.swap(b);
        BOOST_CHECK(a.empty());
        equivalent.test(b);
        a.swap(b);
        equivalent.test(a);
        BOOST_CHECK(b.empty());
    }
    
    {
        X u;
        X& r = u;
        
        check_return_type<X>::equals_ref(r = a);
        equivalent.test(r);
    }
    
    {
        check_return_type<size_type>::equals(a.size());
        BOOST_CHECK_EQUAL(a.size(), (size_type) std::distance(a.begin(), a.end()));
    }

    {
        check_return_type<size_type>::equals(a.max_size());
        // TODO: Check that a.max_size() == size of the largest possible container
        // How do I do that? test::allocator checks that allocations don't exceed
        // that allocator's maximum size. Could check that max_size() works, and
        // that max_size() + 1 doesn't. Only practicle for small max_size though -
        // and it might be possible to implement unordered containers such that
        // max_size() > alloc.max_size(). Or is it?
    }

    {
        check_return_type<bool>::convertible(a.empty());
        BOOST_CHECK_EQUAL(a.empty(), a.size() == 0);
    }

    // 23.1/7
    {
        if(a.empty())
            BOOST_CHECK(a.begin() == a.end());
    }

    // 23.1/8
    {
        iterator i = a.begin(), j = a.end();
        const_iterator ci = a.begin(), cj = a.end();

        if(a.empty()) {
            BOOST_CHECK(i == j);
            BOOST_CHECK(i == cj);
            BOOST_CHECK(ci == j);
            BOOST_CHECK(ci == cj);
            
            BOOST_CHECK(!(i != j));
            BOOST_CHECK(!(i != cj));
            BOOST_CHECK(!(ci != j));
            BOOST_CHECK(!(ci != cj));
        }
        else {
            BOOST_CHECK(!(i == j));
            BOOST_CHECK(!(i == cj));
            BOOST_CHECK(!(ci == j));
            BOOST_CHECK(!(ci == cj));
            
            BOOST_CHECK(i != j);
            BOOST_CHECK(i != cj);
            BOOST_CHECK(ci != j);
            BOOST_CHECK(ci != cj);
        }
    }

    // TODO: 23.1/9 - Make sure this is checked for all constructors.
    {
        check_return_type<typename X::allocator_type>::equals(a.get_allocator());
    }

    // TODO: 23.1/11 - Exception safety:
    // No erase function throws an exception.
    // No copy constructor or assignment operator of a returned iterator throws an exception.

    // No swap() function throws an exception unless that exception is thrown by the copy constructor of assignment operator of the container's Compare object.
    // No swap() function invalidates any references, pointers, or iterators referring to the elements of the containers being swapped.
    //
    // TODO: 21.1/12
    //
    // Unless otherwise specified - iterators not invalidated, values not changed.
}

BOOST_AUTO_UNIT_TEST(basic_tests)
{
    // I don't use the normal template mechanism here, as I want to specify the
    // member type explicitly.
    container_tests((boost::unordered_set<int>*) 0, (int*) 0);
    container_tests((boost::unordered_map<int, float>*) 0, (std::pair<int const, float>*) 0);
    container_tests((boost::unordered_multiset<std::string>*) 0, (std::string*) 0);
    container_tests((boost::unordered_multimap<test::member, char*>*) 0,
            (std::pair<test::member const, char*>*) 0);
}

struct test_structure { int* x; };

META_FUNC_TEST_CASE(basic_tests_2, Container)
{

    Container a;
    container_tests2(a);

    {
        test::random_values<Container> values1((std::min)(10u, a.max_size()));
        Container b(values1.begin(), values1.end());
        container_tests2(b);
    }

    {
        test::random_values<Container> values2((std::min)(1000u, a.max_size()));
        Container c(values2.begin(), values2.end());
        container_tests2(c);
    }

    {
        test::random_values<Container> values3((std::min)(100000u, a.max_size()));
        Container d(values3.begin(), values3.end());
        container_tests2(d);
    }
}

AUTO_META_TESTS(
    (basic_tests_2),
    CONTAINER_SEQ
)
