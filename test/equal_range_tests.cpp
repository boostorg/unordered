
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "./helpers/unit_test.hpp"
#include "./helpers/random_values.hpp"

template <class InputIt, class OutputIt, class Condition>
OutputIt copy_if(InputIt begin, InputIt end, OutputIt out, Condition cond)
{
    for(;begin != end; ++begin)
    {
        if(cond(*begin)) {
            *out = *begin;
            ++out;
        }
    }

    return out;
}

template <class Iterator, class RandomValues, class Value>
void compare(std::pair<Iterator, Iterator> const& range,
        RandomValues const& values, Value const& v)
{
    typedef typename RandomValues::value_type value_type;
    typedef std::vector<value_type> value_container;
    value_container range_values(range.first, range.second);
    value_container orig_values;
    copy_if(values.begin(), values.end(), std::back_inserter(orig_values),
            values.key_match(v));

    if(values.is_unique()) {
        if(orig_values.empty()) {
            BOOST_CHECK_EQUAL(range_values.size(), 0u);
        }
        else {
            BOOST_CHECK_EQUAL(range_values.size(), 1u);
            BOOST_CHECK(orig_values.front() == *range_values.begin());
        }
    }
    else {
        std::sort(range_values.begin(), range_values.end());
        std::sort(orig_values.begin(), orig_values.end());
        BOOST_CHECK_EQUAL(range_values.size(), orig_values.size());
        if(range_values.size() == orig_values.size())
            BOOST_CHECK(std::equal(range_values.begin(), range_values.end(),
                    orig_values.begin()));
    }
}

META_FUNC_TEST_CASE(const_test, Container)
{
    test::random_values<Container> values(500);
    typedef test::random_values<Container> random_values;
    typedef typename random_values::iterator iterator;

    Container const x(values.begin(), values.end());

    for(iterator it = values.begin(); it != values.end(); ++it)
    {
        compare(x.equal_range(values.get_key(*it)),
                values, values.get_key(*it));
    }

    typedef typename random_values::value_type value_type;
    test::generator<value_type> generator;

    for(int i = 0; i < 500; ++i)
    {
        value_type v = generator();
        compare(x.equal_range(values.get_key(v)),
                values, values.get_key(v));
    }
}

META_FUNC_TEST_CASE(nonconst_test, Container)
{
    test::random_values<Container> values(500);
    typedef typename test::random_values<Container>::iterator iterator;

    Container const x(values.begin(), values.end());

    for(iterator it = values.begin(); it != values.end(); ++it)
    {
        compare(x.equal_range(values.get_key(*it)),
                values, values.get_key(*it));
    }
}

AUTO_META_TESTS(
    (const_test)(nonconst_test),
    CONTAINER_SEQ
)
