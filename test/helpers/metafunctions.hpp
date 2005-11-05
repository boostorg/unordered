
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_HELPERS_METAFUNCTIONS_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_METAFUNCTIONS_HEADER

#include <boost/config.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

namespace test
{
    struct unordered_set_type { char x[100]; };
    struct unordered_multiset_type { char x[200]; };
    struct unordered_map_type { char x[300]; };
    struct unordered_multimap_type { char x[400]; };

    template <class V, class H, class P, class A>
    unordered_set_type container_type(
            boost::unordered_set<V, H, P, A> const*);
    template <class V, class H, class P, class A>
    unordered_multiset_type container_type(
            boost::unordered_multiset<V, H, P, A> const*);
    template <class K, class M, class H, class P, class A>
    unordered_map_type container_type(
            boost::unordered_map<K, M, H, P, A> const*);
    template <class K, class M, class H, class P, class A>
    unordered_multimap_type container_type(
            boost::unordered_multimap<K, M, H, P, A> const*);

    template <class Container>
    struct is_set
    {
        BOOST_STATIC_CONSTANT(bool, value =
                sizeof(container_type((Container const*)0))
                    == sizeof(unordered_set_type) ||
                sizeof(container_type((Container const*)0))
                    == sizeof(unordered_multiset_type)
                );
    };

    template <class Container>
    struct is_map
    {
        BOOST_STATIC_CONSTANT(bool, value =
                sizeof(container_type((Container const*)0))
                    == sizeof(unordered_map_type) ||
                sizeof(container_type((Container const*)0))
                    == sizeof(unordered_multimap_type)
                );
    };

    template <class Container>
    struct has_unique_keys
    {
        BOOST_STATIC_CONSTANT(bool, value =
                sizeof(container_type((Container const*)0))
                    == sizeof(unordered_set_type) ||
                sizeof(container_type((Container const*)0))
                    == sizeof(unordered_map_type)
                );
    };

    template <class Container>
    struct has_equivalent_keys
    {
        BOOST_STATIC_CONSTANT(bool, value =
                sizeof(container_type((Container const*)0))
                    == sizeof(unordered_multiset_type) ||
                sizeof(container_type((Container const*)0))
                    == sizeof(unordered_multimap_type)
                );
    };

    // Non Const Value Type

    template <class Container>
    struct map_non_const_value_type
    {
        typedef std::pair<
            typename Container::key_type,
            typename Container::mapped_type> type;
    };


    template <class Container>
    struct non_const_value_type
        : boost::mpl::eval_if<is_map<Container>,
            map_non_const_value_type<Container>,
            boost::mpl::identity<typename Container::value_type> >
    {
    };
}

#endif

