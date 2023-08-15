// Copyright (C) 2023 Joaquin M Lopez Munoz
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../helpers/unordered.hpp"

#include "../objects/test.hpp"
#include "../helpers/random_values.hpp"

#include <algorithm>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/config.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <cstddef>

#ifndef BOOST_NO_CXX11_HDR_RANDOM
#include <random>
#endif

namespace {

  template <class Container, typename ArchivePair>
  void serialization_tests(Container*, ArchivePair*, test::random_generator generator)
  {
    typedef typename Container::iterator      iterator;
    typedef std::vector<iterator>             iterator_vector;
    typedef typename ArchivePair::first_type  output_archive;
    typedef typename ArchivePair::second_type input_archive;

    BOOST_LIGHTWEIGHT_TEST_OSTREAM << "serialization_tests1\n";
    {
      Container c;
      iterator it = c.end();
  
      std::ostringstream oss;
      {
        output_archive oa(oss);
        oa << boost::serialization::make_nvp("container", c);
        oa << boost::serialization::make_nvp("iterator", it);
      }

      test::random_values<Container> values(100, generator);
      Container c2(values.begin(), values.end());
      iterator it2 = c2.begin();
      std::istringstream iss(oss.str());
      input_archive ia(iss);
      ia >> boost::serialization::make_nvp("container", c2);
      ia >> boost::serialization::make_nvp("iterator", it2);
      BOOST_TEST(c2.empty());
      BOOST_TEST(it2 == c2.end());
    }

    BOOST_LIGHTWEIGHT_TEST_OSTREAM << "serialization_tests2\n";
    {
      test::random_values<Container> values(100, generator);
      Container c(values.begin(), values.end());
  
      iterator_vector v;
      for (iterator first = c.begin(), last=c.end(); ; ) {
        v.push_back(first);
        if(first == last) break;
        ++first;
      }

#ifndef BOOST_NO_CXX11_HDR_RANDOM
      std::shuffle(v.begin(), v.end(), std::mt19937(4213));
#else
      std::random_shuffle(v.begin(), v.end());
#endif

      std::ostringstream oss;
      {
        output_archive oa(oss);
        oa << boost::serialization::make_nvp("container", c);
        oa << boost::serialization::make_nvp("iterators", v);
      }

      Container c2;
      iterator_vector v2;
      std::istringstream iss(oss.str());
      input_archive ia(iss);
      ia >> boost::serialization::make_nvp("container", c2);
      ia >> boost::serialization::make_nvp("iterators", v2);
      BOOST_TEST_EQ(v.size(), v2.size());

      for (std::size_t i=0; i < v.size(); ++i) {
        iterator it = v[i];
        iterator it2 = v2[i];
        if (it == c.end()) {
          BOOST_TEST(it2 == c2.end());
        }
        else {
          BOOST_TEST(it2 != c2.end());
          BOOST_TEST(*it == *it2);
        }
      }
    }
  }

  using test::default_generator;

  std::pair<
    boost::archive::text_oarchive, boost::archive::text_iarchive>* text_archive;
  std::pair<
    boost::archive::xml_oarchive, boost::archive::xml_iarchive>* xml_archive;

#ifdef BOOST_UNORDERED_FOA_TESTS
  boost::unordered_flat_map<
    test::object, test::object, test::hash, test::equal_to>* test_flat_map;
  boost::unordered_node_map<
    test::object, test::object, test::hash, test::equal_to>* test_node_map;
  boost::unordered_flat_set<
    test::object, test::hash, test::equal_to>* test_flat_set;
  boost::unordered_node_set<
    test::object, test::hash, test::equal_to>* test_node_set;

  UNORDERED_TEST(serialization_tests,
    ((test_flat_map)(test_node_map)(test_flat_set)(test_node_set))
    ((text_archive)(xml_archive))
    ((default_generator)))
#else
  boost::unordered_map<
    test::object, test::object, test::hash, test::equal_to>* test_map;
  boost::unordered_multimap<
    test::object, test::object, test::hash, test::equal_to>* test_multimap;
  boost::unordered_set<
    test::object, test::hash, test::equal_to>* test_set;
  boost::unordered_multiset<
    test::object, test::hash, test::equal_to>* test_multiset;

  UNORDERED_TEST(serialization_tests,
    ((test_map)(test_multimap)(test_set)(test_multiset))
    ((text_archive)(xml_archive))
    ((default_generator)))
#endif
}

RUN_TESTS()
