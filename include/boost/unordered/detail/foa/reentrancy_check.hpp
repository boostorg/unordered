/* Copyright 2023 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_FOA_REENTRANCY_CHECK_HPP
#define BOOST_UNORDERED_DETAIL_FOA_REENTRANCY_CHECK_HPP

#include <boost/assert.hpp>
#include <boost/unordered/detail/foa/annotated_mutex.hpp>
#include <utility>

#if !defined(BOOST_UNORDERED_DISABLE_REENTRANCY_CHECK)&& \
    !defined(BOOST_ASSERT_IS_VOID)
#define BOOST_UNORDERED_REENTRANCY_CHECK
#endif

namespace boost{
namespace unordered{
namespace detail{
namespace foa{

#if defined(BOOST_UNORDERED_REENTRANCY_CHECK)

class entry_trace
{
public:
  entry_trace(const void* px_):px{px_}
  {
    if(px){
      BOOST_ASSERT_MSG(!find(px),"reentrancy not allowed");
      header()=this;
    }
  }

  /* not used but VS in pre-C++17 mode needs to see it for RVO */
  entry_trace(const entry_trace&);

  ~entry_trace(){clear();}

  void clear()
  {
    if(px){
      header()=next;
      px=nullptr;
    }
  }
  
private:
  static entry_trace*& header()
  {
    thread_local entry_trace *pe=nullptr;
    return pe;
  }

  static bool find(const void* px)
  {
    for(auto pe=header();pe;pe=pe->next){
      if(pe->px==px)return true;
    }
    return false;
  }

  const void  *px;
  entry_trace *next=header();
};

template<typename>
struct reentrancy_checked;

template<template <typename> class LockGuard,typename Mutex>
struct BOOST_UNORDERED_SCOPED_CAPABILITY reentrancy_checked<LockGuard<Mutex>>
{
  reentrancy_checked(const void* px,Mutex& m_)
    BOOST_UNORDERED_ACQUIRE(m_):
    tr{px},lck{m_},m{m_}{}

  reentrancy_checked(const reentrancy_checked& x) BOOST_UNORDERED_ACQUIRE(x.m);

  ~reentrancy_checked() BOOST_UNORDERED_RELEASE() = default;

  void unlock() BOOST_UNORDERED_RELEASE()
  {
    lck.unlock();
    tr.clear();
  }

  entry_trace      tr;
  LockGuard<Mutex> lck;
  Mutex&           m;
};

template<typename>
struct reentrancy_bichecked;

template<template <typename> class LockGuard,typename Mutex>
struct BOOST_UNORDERED_SCOPED_CAPABILITY reentrancy_bichecked<LockGuard<Mutex>>
{
  template<typename Mutex1, typename Mutex2>
  reentrancy_bichecked(const void* px,const void* py,Mutex1& m1_,Mutex2& m2_)
    BOOST_UNORDERED_ACQUIRE(m1_,m2_):
    tr1{px},tr2{py!=px?py:nullptr},lck{m1_,m2_},m1{m1_},m2{m2_}{}

  reentrancy_bichecked(const reentrancy_bichecked& x)
    BOOST_UNORDERED_ACQUIRE(x.m1,x.m2);

  ~reentrancy_bichecked() BOOST_UNORDERED_RELEASE() = default;

  void unlock() BOOST_UNORDERED_RELEASE()
  {
    lck.unlock();
    tr2.clear();
    tr1.clear();
  }

  entry_trace       tr1,tr2;
  LockGuard<Mutex>  lck;
  Mutex            &m1,&m2;
};

#else

template<typename>
struct reentrancy_checked;

template<template <typename> class LockGuard,typename Mutex>
struct BOOST_UNORDERED_SCOPED_CAPABILITY reentrancy_checked<LockGuard<Mutex>>
{
  reentrancy_checked(const void*,Mutex& m_)
    BOOST_UNORDERED_ACQUIRE(m_):
    lck{m_},m{m_}{}

  reentrancy_checked(const reentrancy_checked& x) BOOST_UNORDERED_ACQUIRE(x.m);

  ~reentrancy_checked() BOOST_UNORDERED_RELEASE() = default;

  void unlock() BOOST_UNORDERED_RELEASE()
  {
    lck.unlock();
  }

  LockGuard<Mutex> lck;
  Mutex&           m;
};

template<typename>
struct reentrancy_bichecked;

template<template <typename> class LockGuard,typename Mutex>
struct BOOST_UNORDERED_SCOPED_CAPABILITY reentrancy_bichecked<LockGuard<Mutex>>
{
  template<typename Mutex1, typename Mutex2>
  reentrancy_bichecked(const void*,const void*,Mutex1& m1_,Mutex2& m2_)
    BOOST_UNORDERED_ACQUIRE(m1_,m2_):
    lck{m1_,m2_},m1{m1_},m2{m2_}{}

  reentrancy_bichecked(const reentrancy_bichecked& x)
    BOOST_UNORDERED_ACQUIRE(x.m1,x.m2);

  ~reentrancy_bichecked() BOOST_UNORDERED_RELEASE() = default;

  void unlock() BOOST_UNORDERED_RELEASE()
  {
    lck.unlock();
  }

  LockGuard<Mutex>  lck;
  Mutex            &m1,&m2;
};

#endif

} /* namespace foa */
} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#endif
