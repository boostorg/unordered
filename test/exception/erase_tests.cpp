
//  Copyright Daniel James 2006. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/test/exception_safety.hpp>
#include "../helpers/random_values.hpp"
#include "../helpers/invariants.hpp"
#include "../helpers/helpers.hpp"

template <class T>
struct erase_test_base : public test::exception_base
{
    test::random_values<T> values;
    erase_test_base(unsigned int count = 5) : values(count) {}

    typedef T data_type;

    data_type init() const {
        return T(values.begin(), values.end());
    }

    void check(T const& x) const {
        // TODO: Check that exception was thrown by hash or predicate object?
        test::check_equivalent_keys(x);
    }
};

template <class T>
struct erase_by_key_test1 : public erase_test_base<T>
{
    void run(T& x) const
    {
        typedef typename test::random_values<T>::const_iterator iterator;

        for(iterator it = this->values.begin(), end = this->values.end();
                it != end; ++it)
        {
            x.erase(test::get_key<T>(*it));
        }
    }
};

// TODO: More tests...
// Better test by key.
// Test other erase signatures - generally they won't throw, but the standard
// does allow them to. And test clear as well.

RUN_EXCEPTION_TESTS(
    (erase_by_key_test1),
    CONTAINER_SEQ)
