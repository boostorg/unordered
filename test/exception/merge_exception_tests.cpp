#include "../helpers/exception_test.hpp"
#include "../helpers/invariants.hpp"
#include "../helpers/metafunctions.hpp"
#include "../helpers/random_values.hpp"
#include "./containers.hpp"

template <typename T1, typename T2> void merge_exception_test(T1 x, T2 y)
{
    std::size_t size = x.size() + y.size();

    try {
        ENABLE_EXCEPTIONS;
        x.merge(y);
    } catch (...) {
        test::check_equivalent_keys(x);
        test::check_equivalent_keys(y);
        throw;
    }

    // Not a full check, just want to make sure the merge completed.
    BOOST_TEST(size == x.size() + y.size());
    if (y.size()) {
        BOOST_TEST(test::has_unique_keys<T1>::value);
        for (typename T2::iterator it = y.begin(); it != y.end(); ++it) {
            BOOST_TEST(x.find(test::get_key<T2>(*it)) != x.end());
        }
    }
    test::check_equivalent_keys(x);
    test::check_equivalent_keys(y);
}

template <typename T1, typename T2>
void merge_exception_test(T1 const*, T2 const*, std::size_t count1,
    std::size_t count2, int tag1, int tag2, test::random_generator gen1,
    test::random_generator gen2)
{
    test::random_values<T1> v1(count1, gen1);
    test::random_values<T2> v2(count2, gen2);
    T1 x(v1.begin(), v1.end(), 0, test::exception::hash(tag1),
        test::exception::equal_to(tag1));
    T2 y(v2.begin(), v2.end(), 0, test::exception::hash(tag2),
        test::exception::equal_to(tag2));

    EXCEPTION_LOOP(merge_exception_test(x, y))
}

boost::unordered_set<test::exception::object, test::exception::hash,
    test::exception::equal_to,
    test::exception::allocator<test::exception::object> >* test_set_;
boost::unordered_multiset<test::exception::object, test::exception::hash,
    test::exception::equal_to,
    test::exception::allocator<test::exception::object> >* test_multiset_;
boost::unordered_map<test::exception::object, test::exception::object,
    test::exception::hash, test::exception::equal_to,
    test::exception::allocator2<test::exception::object> >* test_map_;
boost::unordered_multimap<test::exception::object, test::exception::object,
    test::exception::hash, test::exception::equal_to,
    test::exception::allocator2<test::exception::object> >* test_multimap_;

using test::default_generator;
using test::generate_collisions;
using test::limited_range;

// clang-format off
UNORDERED_TEST(merge_exception_test,
    ((test_set_)(test_multiset_))
    ((test_set_)(test_multiset_))
    ((0)(10)(100))
    ((0)(10)(100))
    ((0)(1)(2))
    ((0)(1)(2))
    ((default_generator)(limited_range))
    ((default_generator)(limited_range))
)
UNORDERED_TEST(merge_exception_test,
    ((test_map_)(test_multimap_))
    ((test_map_)(test_multimap_))
    ((0)(10)(100))
    ((0)(10)(100))
    ((0)(1)(2))
    ((0)(1)(2))
    ((default_generator)(limited_range))
    ((default_generator)(limited_range))
)
// Run fewer generate_collisions tests, as they're slow.
UNORDERED_TEST(merge_exception_test,
    ((test_set_)(test_multiset_))
    ((test_set_)(test_multiset_))
    ((10))
    ((10))
    ((0)(1)(2))
    ((0)(1)(2))
    ((generate_collisions))
    ((generate_collisions))
)
UNORDERED_TEST(merge_exception_test,
    ((test_map_)(test_multimap_))
    ((test_map_)(test_multimap_))
    ((10))
    ((10))
    ((0)(1)(2))
    ((0)(1)(2))
    ((generate_collisions))
    ((generate_collisions))
)
// clang-format on

RUN_TESTS_QUIET()
