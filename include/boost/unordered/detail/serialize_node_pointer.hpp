/* Copyright 2023 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_SERIALIZE_NODE_POINTER_HPP
#define BOOST_UNORDERED_DETAIL_SERIALIZE_NODE_POINTER_HPP

#include <boost/core/pointer_traits.hpp>
#include <boost/core/serialization.hpp>
#include <boost/throw_exception.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/unordered/detail/bad_archive_exception.hpp>

namespace boost{
namespace unordered{
namespace detail{

/* Node pointer serialization to support iterator serialization as described
 * in serialize_container.hpp. The underlying technique is to reinterpret_cast
 * Node pointers to serialization_tracker<Node> pointers, which, when
 * dereferenced and serialized, do not emit any serialization payload to the
 * archive, but activate object tracking on the relevant addresses for later
 * use with serialize_node_pointer().
 */

template<typename Node>
struct serialization_tracker
{
  /* An attempt to construct a serialization_tracker means a stray address
   * in the archive, that is, one without a previously tracked node.
   */
  serialization_tracker(){throw_exception(bad_archive_exception());}

  template<typename Archive>
  void serialize(Archive&,unsigned int){} /* no data emitted */
};

template<typename Archive,typename NodePtr>
void track_node_pointer(Archive& ar,NodePtr p)
{
  typedef typename boost::pointer_traits<NodePtr> ptr_traits;
  typedef typename boost::remove_const<
    typename ptr_traits::element_type>::type      node_type;

  if(p){
    ar&core::make_nvp(
      "node",
      *reinterpret_cast<serialization_tracker<node_type>*>(
        const_cast<node_type*>(
          boost::to_address(p))));
  }
}

template<typename Archive,typename NodePtr>
void serialize_node_pointer(
  Archive& ar,NodePtr& p,boost::true_type /* save */)
{
  typedef typename boost::pointer_traits<NodePtr> ptr_traits;
  typedef typename boost::remove_const<
    typename ptr_traits::element_type>::type      node_type;
  typedef serialization_tracker<node_type>        tracker;

  tracker* pt=
    const_cast<tracker*>(
      reinterpret_cast<const tracker*>(
        const_cast<const node_type*>(
          boost::to_address(p))));
  ar<<core::make_nvp("pointer",pt);
}

template<typename Archive,typename NodePtr>
void serialize_node_pointer(
  Archive& ar,NodePtr& p,boost::false_type /* load */)
{
  typedef typename boost::pointer_traits<NodePtr> ptr_traits;
  typedef typename boost::remove_const<
    typename ptr_traits::element_type>::type      node_type;
  typedef serialization_tracker<node_type>        tracker;

  tracker* pt;
  ar>>core::make_nvp("pointer",pt);
  node_type* pn=const_cast<node_type*>(
    reinterpret_cast<const node_type*>(
      const_cast<const tracker*>(pt)));
  p=pn?ptr_traits::pointer_to(*pn):0;
}

template<typename Archive,typename NodePtr>
void serialize_node_pointer(Archive& ar,NodePtr& p)
{
  serialize_node_pointer(
    ar,p,
    boost::integral_constant<bool,Archive::is_saving::value>());
}

} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#endif
