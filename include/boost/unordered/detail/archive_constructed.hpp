/* Copyright 2023 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_ARCHIVE_CONSTRUCTED_HPP
#define BOOST_UNORDERED_DETAIL_ARCHIVE_CONSTRUCTED_HPP

#include <boost/config.hpp>
#include <boost/core/addressof.hpp>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/core/serialization.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp> 

namespace boost{
namespace unordered{
namespace detail{

/* constructs a stack-based object from a serialization archive */

template<typename T>
struct archive_constructed:private noncopyable
{
  template<class Archive>
  archive_constructed(const char* name,Archive& ar,unsigned int version)
  {
    core::load_construct_data_adl(ar,boost::addressof(get()),version);
    BOOST_TRY{
      ar>>core::make_nvp(name,get());
    }
    BOOST_CATCH(...){
      get().~T();
      BOOST_RETHROW;
    }
    BOOST_CATCH_END
  }

  ~archive_constructed()
  {
    get().~T();
  }

#if defined(BOOST_GCC)&&(BOOST_GCC>=4*10000+6*100)
#define BOOST_UNORDERED_IGNORE_WSTRICT_ALIASING
#endif

#if defined(BOOST_UNORDERED_IGNORE_WSTRICT_ALIASING)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

  T& get(){return *reinterpret_cast<T*>(&space);}

#if defined(BOOST_UNORDERED_IGNORE_WSTRICT_ALIASING)
#pragma GCC diagnostic pop
#undef BOOST_UNORDERED_IGNORE_WSTRICT_ALIASING
#endif

private:
  typename aligned_storage<sizeof(T),alignment_of<T>::value>::type space;
};

} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#endif
