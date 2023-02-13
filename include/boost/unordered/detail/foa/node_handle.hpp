/* Copyright 2023 Christian Mazakas.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_FOA_NODE_HANDLE_HPP
#define BOOST_UNORDERED_DETAIL_FOA_NODE_HANDLE_HPP

#include <boost/config.hpp>
#include <boost/core/allocator_access.hpp>

namespace boost{
namespace unordered{
namespace detail{
namespace foa{

template <class Iterator,class NodeType>
struct insert_return_type
{
  Iterator position;
  bool     inserted;
  NodeType node;
};

template <class T>
union opt_storage {
  BOOST_ATTRIBUTE_NO_UNIQUE_ADDRESS T t_;

  opt_storage(){}
  ~opt_storage(){}
};

template <class TypePolicy,class Allocator>
struct node_handle_base
{
  protected:
    using type_policy=TypePolicy;
    using element_type=typename type_policy::element_type;

  public:
    using allocator_type = Allocator;

  private:
    using node_value_type=typename type_policy::value_type;
    node_value_type* p_=nullptr;
    BOOST_ATTRIBUTE_NO_UNIQUE_ADDRESS opt_storage<Allocator> a_;

  protected:
    node_value_type& element()noexcept
    {
      BOOST_ASSERT(!empty());
      return *p_;
    }

    node_value_type const& element()const noexcept
    {
      BOOST_ASSERT(!empty());
      return *p_;
    }

    Allocator& al()noexcept
    {
      BOOST_ASSERT(!empty());
      return a_.t_;
    }

    Allocator const& al()const noexcept
    {
      BOOST_ASSERT(!empty());
      return a_.t_;
    }

    void emplace(node_value_type* p,Allocator a)
    {
      BOOST_ASSERT(empty());
      p_=p;
      new(&a_.t_)Allocator(a);
    }

    void emplace(element_type&& x,Allocator a)
    {
      emplace(x.p,a);
      x.p=nullptr;
    }

    void clear()
    {
      al().~Allocator();
      p_=nullptr;
    }

  public:
    constexpr node_handle_base()noexcept{}

    node_handle_base(node_handle_base&& nh) noexcept
    {
      if (!nh.empty()){
        emplace(nh.p_,nh.al());
        nh.clear();
      }
    }

    node_handle_base& operator=(node_handle_base&& nh)noexcept
    {
      bool const pocma=
        boost::allocator_propagate_on_container_move_assignment<
          Allocator>::type::value;

      BOOST_ASSERT(
        pocma
        ||empty()
        ||nh.empty()
        ||(al()==nh.al()));

      if(!empty()){
        type_policy::destroy(al(),p_);
        if (pocma&&!nh.empty()){al()=std::move(nh.al());}
      }

      if(!nh.empty()){
        if(empty()){new(&a_.t_)Allocator(std::move(nh.al()));}
        p_=nh.p_;

        nh.p_=nullptr;
        nh.a_.t_.~Allocator();
      }else if (!empty()){
        a_.t_.~Allocator();
        p_=nullptr;
      }

      return *this;
    }

    ~node_handle_base()
    {
      if(!empty()){
        type_policy::destroy(al(),p_);
        a_.t_.~Allocator();
      }
    }

    allocator_type get_allocator()const noexcept{return al();}
    explicit operator bool()const noexcept{ return !empty();}
    BOOST_ATTRIBUTE_NODISCARD bool empty()const noexcept{return p_==nullptr;}

    void swap(node_handle_base& nh) noexcept(
      boost::allocator_is_always_equal<Allocator>::type::value||
      boost::allocator_propagate_on_container_swap<Allocator>::type::value)
    {
      using std::swap;

      bool const pocs=
        boost::allocator_propagate_on_container_swap<Allocator>::type::value;

      if (!empty()&&!nh.empty()){
        BOOST_ASSERT(pocs || al()==nh.al());

        node_value_type *p=p_;
        p_=nh.p_;
        nh.p_=p;

        if(pocs){
          swap(al(),nh.al());
        }

        return;
      }

      if (empty()&&nh.empty()){return;}

      if (empty()){
        emplace(nh.p_,nh.al());
        nh.clear();
      }else{
        nh.emplace(p_,al());
        clear();
      }
    }

    friend
    void swap(node_handle_base& lhs,node_handle_base& rhs)
      noexcept(noexcept(lhs.swap(rhs)))
    {
      return lhs.swap(rhs);
    }
};

}
}
}
}

#endif // BOOST_UNORDERED_DETAIL_FOA_NODE_HANDLE_HPP
