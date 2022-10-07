/* Hash mixer for boost::unordered::unordered_flat_[map|set].
 *
 * Copyright 2022 Joaquin M Lopez Munoz.
 * Copyright 2022 Peter Dimov.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_FOA_MIXER_HPP
#define BOOST_UNORDERED_DETAIL_FOA_MIXER_HPP

#include <boost/config.hpp>
#include <boost/container_hash/hash_fwd.hpp>
#include <boost/core/empty_value.hpp>
#include <boost/cstdint.hpp>
#include <climits>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace boost{
namespace unordered{
namespace detail{
namespace foa{

/* mixer<Hash> is functionally equivalent to Hash except if Hash is
 * boost::hash<T> for any of the Ts where boost::hash implements a
 * trivial hashing function not fit for open-addressing hash container:
 * in these cases, the result of boost::hash is post-mixed using
 * 
 *   - 64 bits: xmx (TODO: Peter Dimov to explain)
 *   - 32 bits: xmx33 (TODO: Peter Dimov to explain)
 */

#if defined(BOOST_GCC)
/* GCC's -Wshadow triggers at scenarios like this: 
 *
 *   struct foo{};
 *   template<typename Base>
 *   struct derived:Base
 *   {
 *     void f(){int foo;}
 *   };
 * 
 *   derived<foo>x;
 *   x.f(); // declaration of "foo" in derived::f shadows base type "foo"
 *
 * This makes shadowing warnings unavoidable in general when a class template
 * derives from a user-provided class, as is the case with mixer_impl.
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

template<typename Hash,typename MixPolicy>
class mixer_impl:empty_value<Hash>
{
public:
  using base=empty_value<Hash>;

#if BOOST_CXX_VERSION<201703L
  using argument_type=typename Hash::argument_type;
  using result_type=std::size_t;
#endif

  mixer_impl()=default;
  template<typename... Args>
  mixer_impl(Args&&... args):base{empty_init,std::forward<Args>(args)...}{}

  Hash&       get_base()noexcept{return base::get();}
  const Hash& get_base()const noexcept{return base::get();}

  template<
    typename Key,
    typename std::enable_if<
      std::is_same<
        std::size_t,
        decltype(std::declval<const Hash>()(std::declval<const Key&>()))
      >::value
    >::type* =nullptr
  >
  std::size_t operator()(const Key& x)const
    noexcept(noexcept(std::declval<const Hash>()(x)))
  {
    return MixPolicy::mix(get_base()(x));
  }

  friend void swap(mixer_impl& x,mixer_impl& y)
  {
    using std::swap;
    swap(x.get_base(),y.get_base());
  }
};

#if defined(BOOST_GCC)
#pragma GCC diagnostic pop /* ignored "-Wshadow" */
#endif

struct no_mix
{
  static inline std::size_t mix(std::size_t x)noexcept{return x;}
};

#if defined(SIZE_MAX)
#if ((((SIZE_MAX >> 16) >> 16) >> 16) >> 15) != 0
#define BOOST_UNORDERED_64B_ARCHITECTURE /* >64 bits assumed as 64 bits */
#endif
#elif defined(UINTPTR_MAX) /* used as proxy for std::size_t */
#if ((((UINTPTR_MAX >> 16) >> 16) >> 16) >> 15) != 0
#define BOOST_UNORDERED_64B_ARCHITECTURE
#endif
#endif

struct xmx_mix
{
  static inline std::size_t mix(std::size_t x)noexcept
  {
#if defined(BOOST_UNORDERED_64B_ARCHITECTURE)

    boost::uint64_t z=x;

    z^=z>> 23;
    z*=0xff51afd7ed558ccdull;
    z^=z>>23;

    return (std::size_t)z;

#else /* 32 bits assumed */

    x^=x>>18;
    x*=0x56b5aaadu;
    x^=x>>16;

    return x;

#endif
  }
};

#ifdef BOOST_UNORDERED_64B_ARCHITECTURE
#undef BOOST_UNORDERED_64B_ARCHITECTURE
#endif

template<typename Hash> struct is_boost_hash:std::false_type{};
template<typename Key> struct is_boost_hash<boost::hash<Key>>:std::true_type{};

template<typename Hash> struct boost_hash_key_impl{using type=void;};
template<typename Key> struct boost_hash_key_impl<boost::hash<Key>>
{
  using type=Key;
};
template<typename Hash> using boost_hash_key=
  typename boost_hash_key_impl<Hash>::type;

template<typename Hash>
using mixer=typename std::conditional<
  is_boost_hash<Hash>::value&&(
    std::is_integral<boost_hash_key<Hash>>::value||
    std::is_enum<boost_hash_key<Hash>>::value||
    std::is_floating_point<boost_hash_key<Hash>>::value|| // TODO: not sure about this one
    std::is_pointer<boost_hash_key<Hash>>::value),
  mixer_impl<Hash,xmx_mix>,
  mixer_impl<Hash,no_mix>
>::type;

} /* namespace foa */
} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#endif
