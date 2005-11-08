
//  Copyright Daniel James 2005. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <string>
#include <boost/type_traits/is_same.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/mpl/assert.hpp>
#include "./helpers/functional.hpp"

template <class Container, class Iterator>
struct iterator_checks
{
    typedef Iterator iterator;

    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_value<iterator>::type,
            typename Container::value_type>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_reference<iterator>::type,
            typename Container::reference>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_pointer<iterator>::type,
            typename Container::pointer>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_difference<iterator>::type,
            typename Container::difference_type>));
};

template <class Container, class ConstIterator>
struct const_iterator_checks
{
    typedef ConstIterator const_iterator;

    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_value<const_iterator>::type,
            typename Container::value_type>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_reference<const_iterator>::type,
            typename Container::const_reference>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_pointer<const_iterator>::type,
            typename Container::const_pointer>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_difference<const_iterator>::type,
            typename Container::difference_type>));
};

template <class Container, class Key, class Hash,
         class Pred, class AllocatorType>
struct unordered_typedef_checks
{
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;
    typedef typename Container::local_iterator local_iterator;
    typedef typename Container::const_local_iterator const_local_iterator;

    // 6.3.1/9 rows 1-3
    BOOST_MPL_ASSERT((boost::is_same<
                Key,
                typename Container::key_type>));
    BOOST_MPL_ASSERT((boost::is_same<
                Hash,
                typename Container::hasher>));
    BOOST_MPL_ASSERT((boost::is_same<
                Pred,
                typename Container::key_equal>));

    // 6.3.1/9 rows 4-5
    // TODO: A local_iterator may be used to iterate through a single
    // bucket but may not be used to iterate across buckets.
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::BOOST_ITERATOR_CATEGORY<local_iterator>::type,
            typename boost::BOOST_ITERATOR_CATEGORY<iterator>::type>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_value<local_iterator>::type,
            typename boost::iterator_value<iterator>::type>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_difference<local_iterator>::type,
            typename boost::iterator_difference<iterator>::type>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_pointer<local_iterator>::type,
            typename boost::iterator_pointer<iterator>::type>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_reference<local_iterator>::type,
            typename boost::iterator_reference<iterator>::type>));

    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::BOOST_ITERATOR_CATEGORY<const_local_iterator>::type,
            typename boost::BOOST_ITERATOR_CATEGORY<const_iterator>::type>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_value<const_local_iterator>::type,
            typename boost::iterator_value<const_iterator>::type>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_difference<const_local_iterator>::type,
            typename boost::iterator_difference<const_iterator>::type>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_pointer<const_local_iterator>::type,
            typename boost::iterator_pointer<const_iterator>::type>));
    BOOST_MPL_ASSERT((boost::is_same<
            typename boost::iterator_reference<const_local_iterator>::type,
            typename boost::iterator_reference<const_iterator>::type>));

    // TODO: Is this ever specified?
    BOOST_MPL_ASSERT((boost::is_same<
                AllocatorType,
                typename Container::allocator_type>));
    BOOST_MPL_ASSERT((boost::is_same<
                typename AllocatorType::pointer,
                typename Container::pointer>));
    BOOST_MPL_ASSERT((boost::is_same<
                typename AllocatorType::const_pointer,
                typename Container::const_pointer>));
    BOOST_MPL_ASSERT((boost::is_same<
                typename AllocatorType::reference,
                typename Container::reference>));
    BOOST_MPL_ASSERT((boost::is_same<
                typename AllocatorType::const_reference,
                typename Container::const_reference>));
};

template <class Container, class Key, class T, class Hash, class Pred, class AllocatorType>
struct unordered_map_checks 
{
    unordered_typedef_checks<Container, Key, Hash, Pred, AllocatorType> c1;
    iterator_checks<Container, typename Container::iterator> c2;
    const_iterator_checks<Container, typename Container::const_iterator> c3;
    iterator_checks<Container, typename Container::local_iterator> c4;
    const_iterator_checks<Container, typename Container::const_local_iterator> c5;

    // 6.3.1/7
    BOOST_MPL_ASSERT((boost::is_same<
                typename Container::value_type,
                std::pair<const typename Container::key_type, T> >));
};

template <class Container, class V, class Hash, class Pred, class AllocatorType>
struct unordered_set_checks
{
    unordered_typedef_checks<Container, V, Hash, Pred, AllocatorType> c1;
    const_iterator_checks<Container, typename Container::iterator> c2;
    const_iterator_checks<Container, typename Container::const_iterator> c3;
    const_iterator_checks<Container, typename Container::local_iterator> c4;
    const_iterator_checks<Container, typename Container::const_local_iterator> c5;

    // 6.3.1/7
    BOOST_MPL_ASSERT((boost::is_same<
                typename Container::value_type,
                typename Container::key_type>));
};

unordered_set_checks<
    boost::unordered_multiset<int>, int,
    boost::hash<int>, std::equal_to<int>, std::allocator<int>
> int_multiset_check;

unordered_set_checks<
    boost::unordered_multiset<std::string, test::hash, test::less>,
    std::string, test::hash, test::less, std::allocator<std::string>
> custom_string_multiset_check;

unordered_set_checks<
    boost::unordered_set<int>, int,
    boost::hash<int>, std::equal_to<int>, std::allocator<int>
> int_set_check;

unordered_set_checks<
    boost::unordered_set<std::string, test::hash, test::less>,
    std::string, test::hash, test::less, std::allocator<std::string>
> custom_string_set_check;

unordered_map_checks<
    boost::unordered_map<std::string, int>, std::string, int,
    boost::hash<std::string>, std::equal_to<std::string>,
    std::allocator<std::pair<std::string const, int> >
> string_int_check;

unordered_map_checks<
    boost::unordered_map<char const*, std::string, test::hash, test::less>,
    char const*, std::string,
    test::hash, test::less,
    std::allocator<std::pair<char const* const, std::string> >
> custom_check;

unordered_map_checks<
    boost::unordered_multimap<int, int>,
    int, int,
    boost::hash<int>, std::equal_to<int>,
    std::allocator<std::pair<int const, int> >
> int_int_multi_check;

unordered_map_checks<
    boost::unordered_multimap<std::string, int>,
    std::string, int,
    boost::hash<std::string>, std::equal_to<std::string>,
    std::allocator<std::pair<std::string const, int> >
> string_int_multi_check;

unordered_map_checks<
    boost::unordered_multimap<float, std::string, test::hash, test::less>,
    float, std::string,
    test::hash, test::less,
    std::allocator<std::pair<float const, std::string> >
> custom_multi_check;
