
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_STRONG_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_STRONG_HEADER

#include <boost/shared_ptr.hpp>
#include "./exception_trigger.hpp"

namespace test
{
    class strong_tester
    {
        mutable bool dismissed_;
    protected:
        strong_tester();
    public:
        virtual ~strong_tester();
        void dismiss() const;
        bool is_dismissed() const;
        void call_test();
        virtual void test() = 0;
    };

    template <class T>
    class default_strong_tester
        : public strong_tester
    {
        T const& reference;
        T copy;
    public:
        default_strong_tester(T const& x) : reference(x), copy(x) {}

        void test()
        {
            BOOST_CHECK(reference == copy);
        }
    };

    typedef boost::shared_ptr<strong_tester> strong_tester_ptr;

    //template <class T>
    //strong_tester_ptr create_tester_impl(T const& x, float)
    //{
    //    return strong_tester_ptr(new default_strong_tester<T>(x));
    //}

    template <class T>
    strong_tester_ptr create_tester(T const& x)
    {
        DEACTIVATE_EXCEPTIONS;
        return create_tester_impl(x, 0);
    }

    class strong_test_holder
    {
        strong_tester_ptr ptr_;
    public:
        strong_test_holder(strong_tester_ptr const&);
        ~strong_test_holder();
        bool is_dismissed() const;
        void dismiss();
    private:
        strong_test_holder(strong_test_holder const&);
        strong_test_holder& operator=(strong_test_holder const&);
    };
}

#define STRONG_TEST_ANON(x) \
    STRONG_TEST(BOOST_PP_CAT(STRONG_TEST_tester, __LINE__), x)

#define STRONG_TEST(tester, x) \
    for(::test::strong_test_holder tester(::test::create_tester(x)); \
            !tester.is_dismissed(); tester.dismiss())

#endif
