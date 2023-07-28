/* Copyright 2023 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_FOA_ANNOTATED_MUTEX_HPP
#define BOOST_UNORDERED_DETAIL_FOA_ANNOTATED_MUTEX_HPP

#include <boost/config.hpp>
#include <utility>

namespace boost{
namespace unordered{
namespace detail{
namespace foa{

/* reference: https://clang.llvm.org/docs/ThreadSafetyAnalysis.htm */

#if defined(BOOST_CLANG)&&!defined(SWIG)
#define BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(x) __attribute__((x))
#else
#define BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(x) 
#endif

#define BOOST_UNORDERED_CAPABILITY(x) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(capability(x))

#define BOOST_UNORDERED_SCOPED_CAPABILITY \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(scoped_lockable)

#define BOOST_UNORDERED_GUARDED_BY(x) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(guarded_by(x))

#define BOOST_UNORDERED_PT_GUARDED_BY(x) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(pt_guarded_by(x))

#define BOOST_UNORDERED_ACQUIRED_BEFORE(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(acquired_before(__VA_ARGS__))

#define BOOST_UNORDERED_ACQUIRED_AFTER(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(acquired_after(__VA_ARGS__))

#define BOOST_UNORDERED_REQUIRES(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(requires_capability(__VA_ARGS__))

#define BOOST_UNORDERED_REQUIRES_SHARED(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(requires_shared_capability(__VA_ARGS__))

#define BOOST_UNORDERED_ACQUIRE(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(acquire_capability(__VA_ARGS__))

#define BOOST_UNORDERED_ACQUIRE_SHARED(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(acquire_shared_capability(__VA_ARGS__))

#define BOOST_UNORDERED_RELEASE(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(release_capability(__VA_ARGS__))

#define BOOST_UNORDERED_RELEASE_SHARED(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(release_shared_capability(__VA_ARGS__))

#define BOOST_UNORDERED_RELEASE_GENERIC(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(release_generic_capability(__VA_ARGS__))

#define BOOST_UNORDERED_TRY_ACQUIRE(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(try_acquire_capability(__VA_ARGS__))

#define BOOST_UNORDERED_TRY_ACQUIRE_SHARED(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(         \
  try_acquire_shared_capability(__VA_ARGS__))

#define BOOST_UNORDERED_EXCLUDES(...) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(locks_excluded(__VA_ARGS__))

#define BOOST_UNORDERED_ASSERT_CAPABILITY(x) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(assert_capability(x))

#define BOOST_UNORDERED_ASSERT_SHARED_CAPABILITY(x) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(assert_shared_capability(x))

#define BOOST_UNORDERED_RETURN_CAPABILITY(x) \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(lock_returned(x))

#define BOOST_UNORDERED_NO_THREAD_SAFETY_ANALYSIS \
BOOST_UNORDERED_THREAD_ANNOTATION_ATTR(no_thread_safety_analysis)

template<typename Mutex>
struct BOOST_UNORDERED_CAPABILITY("mutex") annotated_mutex:Mutex
{
  using super=Mutex;

  using super::super;

  void lock() noexcept(noexcept(super::lock()))
  BOOST_UNORDERED_ACQUIRE()
  {
    super::lock();
  }

  bool try_lock() noexcept(noexcept(super::try_lock()))
  BOOST_UNORDERED_TRY_ACQUIRE(true)
  {
    return super::try_lock();
  }

  void unlock() noexcept(noexcept(super::unlock()))
  BOOST_UNORDERED_RELEASE()
  {
    super::unlock();
  }

  void lock_shared() noexcept(noexcept(super::lock_shared()))
  BOOST_UNORDERED_ACQUIRE_SHARED()
  {
    super::lock_shared();
  }

  bool try_lock_shared() noexcept(noexcept(super::try_lock_shared()))
  BOOST_UNORDERED_TRY_ACQUIRE_SHARED(true)
  {
    return super::try_lock();
  }

  void unlock_shared() noexcept(noexcept(super::unlock_shared()))
  BOOST_UNORDERED_RELEASE_SHARED()
  {
    super::unlock_shared();
  }
};

} /* namespace foa */
} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#endif
