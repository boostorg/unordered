/* Fast open-addressing hash table.
 *
 * Copyright 2022-2023 Joaquin M Lopez Munoz.
 * Copyright 2023 Christian Mazakas.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_FOA_TABLE_HPP
#define BOOST_UNORDERED_DETAIL_FOA_TABLE_HPP

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/config/workaround.hpp>
#include <boost/unordered/detail/foa/core.hpp>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace boost{
namespace unordered{
namespace detail{
namespace foa{

template<typename Integral>
struct plain_integral
{
  operator Integral()const{return n;}
  void operator=(Integral m){n=m;}

#if BOOST_WORKAROUND(BOOST_GCC,>=50000 && BOOST_GCC<60000)
  void operator|=(Integral m){n=static_cast<Integral>(n|m);}
  void operator&=(Integral m){n=static_cast<Integral>(n&m);}
#else
  void operator|=(Integral m){n|=m;}
  void operator&=(Integral m){n&=m;}
#endif

  Integral n;
};

template<typename,typename,typename,typename>
class table;

/* table_iterator keeps two pointers:
 * 
 *   - A pointer p to the element slot.
 *   - A pointer pc to the n-th byte of the associated group metadata, where n
 *     is the position of the element in the group.
 *
 * A simpler solution would have been to keep a pointer p to the element, a
 * pointer pg to the group, and the position n, but that would increase
 * sizeof(table_iterator) by 4/8 bytes. In order to make this compact
 * representation feasible, it is required that group objects are aligned
 * to their size, so that we can recover pg and n as
 * 
 *   - n = pc%sizeof(group)
 *   - pg = pc-n
 * 
 * (for explanatory purposes pg and pc are treated above as if they were memory
 * addresses rather than pointers).
 * 
 * p = nullptr is conventionally used to mark end() iterators.
 */

/* internal conversion from const_iterator to iterator */
struct const_iterator_cast_tag{}; 

template<typename TypePolicy,typename Group,bool Const>
class table_iterator
{
  using type_policy=TypePolicy;
  using table_element_type=typename type_policy::element_type;
  using group_type=Group;
  static constexpr auto N=group_type::N;
  static constexpr auto regular_layout=group_type::regular_layout;

public:
  using difference_type=std::ptrdiff_t;
  using value_type=typename type_policy::value_type;
  using pointer=
    typename std::conditional<Const,value_type const*,value_type*>::type;
  using reference=
    typename std::conditional<Const,value_type const&,value_type&>::type;
  using iterator_category=std::forward_iterator_tag;
  using element_type=
    typename std::conditional<Const,value_type const,value_type>::type;

  table_iterator()=default;
  template<bool Const2,typename std::enable_if<!Const2>::type* =nullptr>
  table_iterator(const table_iterator<TypePolicy,Group,Const2>& x):
    pc{x.pc},p{x.p}{}
  table_iterator(
    const_iterator_cast_tag, const table_iterator<TypePolicy,Group,true>& x):
    pc{x.pc},p{x.p}{}

  inline reference operator*()const noexcept
    {return type_policy::value_from(*p);}
  inline pointer operator->()const noexcept
    {return std::addressof(type_policy::value_from(*p));}
  inline table_iterator& operator++()noexcept{increment();return *this;}
  inline table_iterator operator++(int)noexcept
    {auto x=*this;increment();return x;}
  friend inline bool operator==(
    const table_iterator& x,const table_iterator& y)
    {return x.p==y.p;}
  friend inline bool operator!=(
    const table_iterator& x,const table_iterator& y)
    {return !(x==y);}

private:
  template<typename,typename,bool> friend class table_iterator;
  template<typename,typename,typename,typename> friend class table;

  table_iterator(Group* pg,std::size_t n,const table_element_type* p_):
    pc{reinterpret_cast<unsigned char*>(const_cast<group_type*>(pg))+n},
    p{const_cast<table_element_type*>(p_)}
    {}

  inline void increment()noexcept
  {
    BOOST_ASSERT(p!=nullptr);
    increment(std::integral_constant<bool,regular_layout>{});
  }

  inline void increment(std::true_type /* regular layout */)noexcept
  {
    for(;;){
      ++p;
      if(reinterpret_cast<uintptr_t>(pc)%sizeof(group_type)==N-1){
        pc+=sizeof(group_type)-(N-1);
        break;
      }
      ++pc;
      if(!group_type::is_occupied(pc))continue;
      if(BOOST_UNLIKELY(group_type::is_sentinel(pc)))p=nullptr;
      return;
    }

    for(;;){
      int mask=reinterpret_cast<group_type*>(pc)->match_occupied();
      if(mask!=0){
        auto n=unchecked_countr_zero(mask);
        if(BOOST_UNLIKELY(reinterpret_cast<group_type*>(pc)->is_sentinel(n))){
          p=nullptr;
        }
        else{
          pc+=n;
          p+=n;
        }
        return;
      }
      pc+=sizeof(group_type);
      p+=N;
    }
  }

  inline void increment(std::false_type /* interleaved */)noexcept
  {
    std::size_t n0=reinterpret_cast<uintptr_t>(pc)%sizeof(group_type);
    pc-=n0;

    int mask=(
      reinterpret_cast<group_type*>(pc)->match_occupied()>>(n0+1))<<(n0+1);
    if(!mask){
      do{
        pc+=sizeof(group_type);
        p+=N;
      }
      while((mask=reinterpret_cast<group_type*>(pc)->match_occupied())==0);
    }

    auto n=unchecked_countr_zero(mask);
    if(BOOST_UNLIKELY(reinterpret_cast<group_type*>(pc)->is_sentinel(n))){
      p=nullptr;
    }
    else{
      pc+=n;
      p-=n0;
      p+=n;
    }
  }

  unsigned char      *pc=nullptr;
  table_element_type *p=nullptr;
};

/* foa::table interface departs in a number of ways from that of C++ unordered
 * associative containers because it's not for end-user consumption
 * (boost::unordered_[flat|node]_[map|set] wrappers complete it as
 * appropriate).
 *
 * The table supports two main modes of operation: flat and node-based. In the
 * flat case, buckets directly store elements. For node-based, buckets store
 * pointers to individually heap-allocated elements.
 *
 * For both flat and node-based:
 *
 *   - begin() is not O(1).
 *   - No bucket API.
 *   - Load factor is fixed and can't be set by the user.
 * 
 * For flat only:
 *
 *   - value_type must be moveable.
 *   - Pointer stability is not kept under rehashing.
 *   - No extract API.
 *
 * try_emplace, erase and find support heterogenous lookup by default, that is,
 * without checking for any ::is_transparent typedefs --the checking is done by
 * boost::unordered_[flat|node]_[map|set].
 */

template <typename TypePolicy,typename Hash,typename Pred,typename Allocator>
using table_core_impl=
  table_core<TypePolicy,group15<plain_integral>,std::size_t,Hash,Pred,Allocator>;

#include <boost/unordered/detail/foa/ignore_wshadow.hpp>

#if defined(BOOST_MSVC)
#pragma warning(push)
#pragma warning(disable:4714) /* marked as __forceinline not inlined */
#endif

template<typename TypePolicy,typename Hash,typename Pred,typename Allocator>
class table:table_core_impl<TypePolicy,Hash,Pred,Allocator>
{
  using super=table_core_impl<TypePolicy,Hash,Pred,Allocator>;
  using type_policy=typename super::type_policy;
  using group_type=typename super::group_type;
  using super::N;
  using prober=typename super::prober;
  using locator=typename super::locator;

public:
  using key_type=typename super::key_type;
  using init_type=typename super::init_type;
  using value_type=typename super::value_type;
  using element_type=typename super::element_type;

private:
  static constexpr bool has_mutable_iterator=
    !std::is_same<key_type,value_type>::value;

public:
  using hasher=typename super::hasher;
  using key_equal=typename super::key_equal;
  using allocator_type=typename super::allocator_type;
  using pointer=typename super::pointer;
  using const_pointer=typename super::const_pointer;
  using reference=typename super::reference;
  using const_reference=typename super::const_reference;
  using size_type=typename super::size_type;
  using difference_type=typename super::difference_type;
  using const_iterator=table_iterator<type_policy,group_type,true>;
  using iterator=typename std::conditional<
    has_mutable_iterator,
    table_iterator<type_policy,group_type,false>,
    const_iterator>::type;

  table(
    std::size_t n=default_bucket_count,const Hash& h_=Hash(),
    const Pred& pred_=Pred(),const Allocator& al_=Allocator()):
    super{n,h_,pred_,al_}
    {}

  table(const table& x)=default;
  table(table&& x)=default;
  table(const table& x,const Allocator& al_):super{x,al_}{}
  table(table&& x,const Allocator& al_):super{std::move(x),al_}{}
  ~table()=default;

  table& operator=(const table& x)=default;
  table& operator=(table&& x)=default;

  using super::get_allocator;

  iterator begin()noexcept
  {
    iterator it{this->arrays.groups,0,this->arrays.elements};
    if(this->arrays.elements&&
       !(this->arrays.groups[0].match_occupied()&0x1))++it;
    return it;
  }

  const_iterator begin()const noexcept
                   {return const_cast<table*>(this)->begin();}
  iterator       end()noexcept{return {};}
  const_iterator end()const noexcept{return const_cast<table*>(this)->end();}
  const_iterator cbegin()const noexcept{return begin();}
  const_iterator cend()const noexcept{return end();}

  using super::empty;
  using super::size;
  using super::max_size;

  template<typename... Args>
  BOOST_FORCEINLINE std::pair<iterator,bool> emplace(Args&&... args)
  {
    auto x=alloc_make_insert_type<type_policy>(
      this->al(),std::forward<Args>(args)...);
    return emplace_impl(type_policy::move(x.value()));
  }

  template<typename Key,typename... Args>
  BOOST_FORCEINLINE std::pair<iterator,bool> try_emplace(
    Key&& x,Args&&... args)
  {
    return emplace_impl(
      try_emplace_args_t{},std::forward<Key>(x),std::forward<Args>(args)...);
  }

  BOOST_FORCEINLINE std::pair<iterator,bool>
  insert(const init_type& x){return emplace_impl(x);}

  BOOST_FORCEINLINE std::pair<iterator,bool>
  insert(init_type&& x){return emplace_impl(std::move(x));}

  /* template<typename=void> tilts call ambiguities in favor of init_type */

  template<typename=void>
  BOOST_FORCEINLINE std::pair<iterator,bool>
  insert(const value_type& x){return emplace_impl(x);}

  template<typename=void>
  BOOST_FORCEINLINE std::pair<iterator,bool>
  insert(value_type&& x){return emplace_impl(std::move(x));}

  template<typename T=element_type>
  BOOST_FORCEINLINE
  typename std::enable_if<
    !std::is_same<T,value_type>::value,
    std::pair<iterator,bool>
  >::type
  insert(element_type&& x){return emplace_impl(std::move(x));}

  template<
    bool dependent_value=false,
    typename std::enable_if<
      has_mutable_iterator||dependent_value>::type* =nullptr
  >
  void erase(iterator pos)noexcept{return erase(const_iterator(pos));}

  BOOST_FORCEINLINE
  void erase(const_iterator pos)noexcept
  {
    super::erase(pos.pc,pos.p);
  }

  template<typename Key>
  BOOST_FORCEINLINE
  auto erase(Key&& x) -> typename std::enable_if<
    !std::is_convertible<Key,iterator>::value&&
    !std::is_convertible<Key,const_iterator>::value, std::size_t>::type
  {
    auto it=find(x);
    if(it!=end()){
      erase(it);
      return 1;
    }
    else return 0;
  }

  void swap(table& x)
    noexcept(noexcept(std::declval<super&>().swap(std::declval<super&>())))
  {
    super::swap(x);
  }

  using super::clear;

  element_type extract(const_iterator pos)
  {
    BOOST_ASSERT(pos!=end());
    erase_on_exit e{*this,pos};
    (void)e;
    return std::move(*pos.p);
  }

  // TODO: should we accept different allocator too?
  template<typename Hash2,typename Pred2>
  void merge(table<TypePolicy,Hash2,Pred2,Allocator>& x)
  {
    x.for_all_elements([&,this](group_type* pg,unsigned int n,element_type* p){
      erase_on_exit e{x,{pg,n,p}};
      if(!emplace_impl(type_policy::move(*p)).second)e.rollback();
    });
  }

  template<typename Hash2,typename Pred2>
  void merge(table<TypePolicy,Hash2,Pred2,Allocator>&& x){merge(x);}

  using super::hash_function;
  using super::key_eq;

  template<typename Key>
  BOOST_FORCEINLINE iterator find(const Key& x)
  {
    auto hash=this->hash_for(x);
    return find_impl(x,this->position_for(hash),hash);
  }

  template<typename Key>
  BOOST_FORCEINLINE const_iterator find(const Key& x)const
  {
    return const_cast<table*>(this)->find(x);
  }

  using super::capacity;
  using super::load_factor;
  using super::max_load_factor;
  using super::max_load;
  using super::rehash;
  using super::reserve;

  template<typename Predicate>
  friend std::size_t erase_if(table& x,Predicate pr)
  {
    return x.erase_if_impl(pr);
  }

private:
  struct erase_on_exit
  {
    erase_on_exit(table& x_,const_iterator it_):x{x_},it{it_}{}
    ~erase_on_exit(){if(!rollback_)x.erase(it);}

    void rollback(){rollback_=true;}

    table&         x;
    const_iterator it;
    bool           rollback_=false;
  };

  static inline iterator make_iterator(const locator& l)noexcept
  {
    return {l.pg,l.n,l.p};
  }

#if defined(BOOST_MSVC)
/* warning: forcing value to bool 'true' or 'false' in bool(pred()...) */
#pragma warning(push)
#pragma warning(disable:4800)
#endif

  template<typename Key>
  BOOST_FORCEINLINE iterator find_impl(
    const Key& x,std::size_t pos0,std::size_t hash)const
  {    
    prober pb(pos0);
    do{
      auto pos=pb.get();
      auto pg=this->arrays.groups+pos;
      auto mask=pg->match(hash);
      if(mask){
        BOOST_UNORDERED_ASSUME(this->arrays.elements!=nullptr);
        auto p=this->arrays.elements+pos*N;
        this->prefetch_elements(p);
        do{
          auto n=unchecked_countr_zero(mask);
          if(BOOST_LIKELY(bool(this->pred()(x,this->key_from(p[n]))))){
            return {pg,n,p+n};
          }
          mask&=mask-1;
        }while(mask);
      }
      if(BOOST_LIKELY(pg->is_not_overflowed(hash))){
        return {}; /* end() */
      }
    }
    while(BOOST_LIKELY(pb.next(this->arrays.groups_size_mask)));
    return {}; /* end() */
  }

#if defined(BOOST_MSVC)
#pragma warning(pop) /* C4800 */
#endif

  template<typename... Args>
  BOOST_FORCEINLINE std::pair<iterator,bool> emplace_impl(Args&&... args)
  {
    const auto &k=this->key_from(std::forward<Args>(args)...);
    auto        hash=this->hash_for(k);
    auto        pos0=this->position_for(hash);
    auto        it=find_impl(k,pos0,hash);

    if(it!=end()){
      return {it,false};
    }
    if(BOOST_LIKELY(this->size_<this->ml)){
      return {
        make_iterator(
          this->unchecked_emplace_at(pos0,hash,std::forward<Args>(args)...)),
        true
      };  
    }
    else{
      return {
        make_iterator(
          this->unchecked_emplace_with_rehash(
            hash,std::forward<Args>(args)...)),
        true
      };  
    }
  }
};

#if defined(BOOST_MSVC)
#pragma warning(pop) /* C4714 */
#endif

#include <boost/unordered/detail/foa/restore_wshadow.hpp>

} /* namespace foa */
} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#endif
