
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_AUTO_UNIT_TEST_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_AUTO_UNIT_TEST_HEADER

#include "./base.hpp"
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each_product.hpp>


#define AUTO_UNIT_TEST(name) \
    AUTO_UNIT_TEST2(name, BOOST_PP_CAT(name##_, impl))

#define AUTO_UNIT_TEST2(name, impl_name) \
    void impl_name(); \
    BOOST_AUTO_UNIT_TEST(name) \
    { \
        impl_name(); \
        ::test::end(); \
    } \
    void impl_name()

#define AUTO_TEMPLATE_TEST(name, T, type_seq) \
    AUTO_TEMPLATE_TEST2(name, BOOST_PP_CAT(name##_, impl), T, type_seq)

#define AUTO_TEMPLATE_TEST2(name, impl_name, T, type_seq) \
    template <class T> \
    void impl_name(); \
    BOOST_PP_SEQ_FOR_EACH(AUTO_TEMPLATE_TEST_OP, name, type_seq) \
    template <class T> \
    void impl_name()

#define AUTO_TEMPLATE_TEST_OP(r, name, type) \
    static boost::unit_test::ut_detail::auto_unit_test_registrar \
        BOOST_PP_CAT(test_registrar_##name##_, type) \
            ( BOOST_TEST_CASE( BOOST_PP_CAT(name##_, impl)<type> ) );

#define META_FUNC_TEST_CASE(name, T) \
    META_FUNC_TEST_CASE2(name, T, BOOST_PP_CAT(name##_, impl))

#define META_FUNC_TEST_CASE2(name, T, impl_name) \
    template <class T> \
    void impl_name(T* = 0); \
    template <class T> \
    void name(T* x = 0) { \
        impl_name(x); \
        ::test::end(); \
    } \
    template <class T> \
    void impl_name(T*)

#define RUN_TEST_OP(r, product) \
    RUN_TEST_OP2( \
        BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(0, product), \
            BOOST_PP_CAT(_, BOOST_PP_SEQ_ELEM(1, product)) \
        ), \
        BOOST_PP_SEQ_ELEM(0, product), \
        BOOST_PP_SEQ_ELEM(1, product) \
    )

#define RUN_TEST_OP2(name, test_func, type) \
	BOOST_AUTO_UNIT_TEST(name) \
    { \
        test_func((type*) 0); \
        ::test::end(); \
    }

#define AUTO_META_TESTS(test_seq, param_seq) \
    BOOST_PP_SEQ_FOR_EACH_PRODUCT(RUN_TEST_OP, (test_seq)(param_seq))
    
#endif
