
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./strong.hpp"
#include <boost/test/test_tools.hpp>

namespace test
{
    strong_tester::strong_tester() : dismissed_(false) {}
    strong_tester::~strong_tester() { BOOST_CHECK(dismissed_); }
    void strong_tester::dismiss() const { dismissed_ = true; }
    bool strong_tester::is_dismissed() const { return dismissed_; }
    void strong_tester::call_test() {
        if(!is_dismissed())
        {
            DEACTIVATE_EXCEPTIONS;
            try {
                test();
            } catch(...) {
                BOOST_ERROR("Exception thrown in strong test.");
            }
            dismissed_ = true;
        }
    }

    strong_test_holder::strong_test_holder(strong_tester_ptr const& x) : ptr_(x) {}
    strong_test_holder::~strong_test_holder() { ptr_->call_test(); }
    bool strong_test_holder::is_dismissed() const { return ptr_->is_dismissed(); }
    void strong_test_holder::dismiss() { ptr_->dismiss(); }
}
