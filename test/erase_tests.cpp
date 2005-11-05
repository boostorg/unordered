
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./containers.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <boost/next_prior.hpp>
#include "./helpers/unit_test.hpp"
#include "./helpers/exception_test.hpp"
#include "./helpers/random_values.hpp"
#include "./helpers/constructors.hpp"
#include "./invariant.hpp"

META_FUNC_TEST_CASE(range_erase_test,Container)
{
    test::constructors<Container> constructor;

    test::random_values<Container> values(100);
    typedef typename Container::iterator iterator;

    {
        Container x(values.begin(), values.end(), 0,
                constructor.hasher(55),
                constructor.key_equal(55),
                constructor.allocator(10));
        const std::size_t size = x.size();
        INVARIANT_CHECK(x);
        iterator pos;

        // Should be no throw.
        ACTIVATE_EXCEPTIONS;

        BOOST_CHECKPOINT("Erase nothing from the beginning");
        BOOST_CHECK(x.begin() == x.erase(x.begin(), x.begin()));
        BOOST_CHECK_EQUAL(x.size(), size);
        test::check_invariants();

        BOOST_CHECKPOINT("Erase nothing from the end");
        BOOST_CHECK(x.end() == x.erase(x.end(), x.end()));
        BOOST_CHECK_EQUAL(x.size(), size);
        test::check_invariants();

        BOOST_CHECKPOINT("Erase nothing from the middle");
        BOOST_CHECK(boost::next(x.begin(), 4) == x.erase(
                    boost::next(x.begin(), 4),
                    boost::next(x.begin(), 4)));
        BOOST_CHECK_EQUAL(x.size(), size);
        test::check_invariants();

        BOOST_CHECKPOINT("Erase 3 from the middle");
        pos = x.erase(boost::next(x.begin(), 1), boost::next(x.begin(), 4));
        BOOST_CHECK(boost::next(x.begin(), 1) == pos);
        BOOST_CHECK_EQUAL(x.size(), size - 3);
        test::check_invariants();

        BOOST_CHECKPOINT("Erase all but the first 1");
        pos = x.erase(boost::next(x.begin(), 1), x.end());
        BOOST_CHECK(x.end() == pos);
        BOOST_CHECK_EQUAL(x.size(), 1u);
        test::check_invariants();
    }

    {
        Container x(values.begin(), values.end());
        const std::size_t size = x.size();
        INVARIANT_CHECK(x);
        iterator pos;

        // Should be no throw.
        ACTIVATE_EXCEPTIONS;

        BOOST_CHECKPOINT("Erase first 2");
        pos = x.erase(x.begin(), boost::next(x.begin(), 2));
        BOOST_CHECK(x.begin() == pos);
        BOOST_CHECK_EQUAL(x.size(), size - 2);
        test::check_invariants();
    }

    {
        Container x(values.begin(), values.end());
        INVARIANT_CHECK(x);
        iterator pos;

        // Should be no throw.
        ACTIVATE_EXCEPTIONS;

        BOOST_CHECKPOINT("Erase all");
        pos = x.erase(x.begin(), x.end());
        BOOST_CHECK(x.begin() == pos && x.end() == pos);
        BOOST_CHECK(x.empty());
        test::check_invariants();
    }
}

META_FUNC_TEST_CASE(erase_by_key_test,Container)
{
    test::constructors<Container> constructor;
    test::sorted_random_values<Container> values(10);

    // Exceptions only from the hash function.
    EXCEPTION_TEST(1000)
    {
        DEACTIVATE_EXCEPTIONS;
        Container x(values.begin(), values.end(), 0,
                constructor.hasher(55),
                constructor.key_equal(55),
                constructor.allocator(10));
        INVARIANT_CHECK(x);

        for(int i = 0; i < 10; i += values.count(values[i])) {
            std::size_t key_count = values.key_count(values[i]);
            {
                ACTIVATE_EXCEPTIONS;
                BOOST_CHECK_EQUAL(key_count,
                            x.erase(values.get_key(values[i])));
            }
            BOOST_CHECK(x.find(values.get_key(values[i])) == x.end());
            BOOST_CHECK_EQUAL(0u, x.erase(values.get_key(values[i])));
        }

        BOOST_CHECK(x.empty());
    }
    EXCEPTION_TEST_END
}

META_FUNC_TEST_CASE(erase_subrange_test,Container)
{
    test::random_values<Container> values(100);
    Container x(values.begin(), values.end());

    // Should be no throw.
    ACTIVATE_EXCEPTIONS;

    typedef typename Container::const_iterator const_iterator;
    typedef typename Container::iterator iterator;

    std::size_t length = x.size();
    std::size_t begin_index = length / 2;
    std::size_t end_index = (length + begin_index) / 2;
    std::size_t sub_begin_index = (end_index - begin_index) / 4;
    std::size_t sub_end_index = sub_begin_index * 3;

    const_iterator begin = boost::next(x.begin(), begin_index);
    const_iterator end = boost::next(x.begin(), end_index);

    iterator pos = x.erase(boost::next(begin, sub_begin_index),
            boost::next(begin, sub_end_index));

    BOOST_CHECK(pos == boost::next(x.begin(), begin_index + sub_begin_index));
    BOOST_CHECK(pos == boost::next(begin, sub_begin_index));
    BOOST_CHECK_EQUAL(
            (end_index - begin_index) - (sub_end_index - sub_begin_index),
            static_cast<std::size_t>(std::distance(begin, end)));

    test::invariant_check(x);
}

META_FUNC_TEST_CASE(erase_by_iterator_test,Container)
{
    test::random_values<Container> values(100);
    Container x(values.begin(), values.end());
    INVARIANT_CHECK(x);
    std::size_t size = x.size();

    typedef typename Container::iterator iterator;

    // Should be no throw.
    ACTIVATE_EXCEPTIONS;

    while(!x.empty()) {
        using namespace std;
        int index = rand() % x.size();
        iterator pos = x.erase(boost::next(x.begin(), index));
        --size;
        BOOST_CHECK_EQUAL(size, x.size());

        BOOST_CHECK(boost::next(x.begin(), index) == pos);
        test::check_invariants();
    }
}

AUTO_META_TESTS(
    (range_erase_test)(erase_by_key_test)(erase_subrange_test)
    (erase_by_iterator_test),
    CONTAINER_SEQ
)
