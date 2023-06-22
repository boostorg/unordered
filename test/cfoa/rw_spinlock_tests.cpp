// Copyright 2023 Peter Dimov
// Copyright 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "helpers.hpp"

#include <boost/unordered/detail/foa/rw_spinlock.hpp>

#ifdef BOOST_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#ifdef BOOST_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif

#include <boost/thread/shared_mutex.hpp>

#ifdef BOOST_GCC
#pragma GCC diagnostic pop
#endif

#ifdef BOOST_CLANG
#pragma GCC diagnostic pop
#endif

#include <mutex>
#include <shared_mutex>
#include <thread>

using boost::unordered::detail::foa::rw_spinlock;

static int count = 0;

UNORDERED_AUTO_TEST (rw_spinlock_test) {
  rw_spinlock sp, sp2;

  sp.lock();
  sp2.lock();
  sp.unlock();
  sp2.unlock();

  {
    std::lock_guard<rw_spinlock> lock(sp);
    std::lock_guard<rw_spinlock> lock2(sp2);
  }
}

UNORDERED_AUTO_TEST (rw_spinlock_test2) {
  rw_spinlock sp, sp2;

  BOOST_TEST(sp.try_lock());
  BOOST_TEST(!sp.try_lock());
  BOOST_TEST(sp2.try_lock());
  BOOST_TEST(!sp.try_lock());
  BOOST_TEST(!sp2.try_lock());
  sp.unlock();
  sp2.unlock();

  sp.lock();
  BOOST_TEST(!sp.try_lock());
  sp2.lock();
  BOOST_TEST(!sp.try_lock());
  BOOST_TEST(!sp2.try_lock());
  sp.unlock();
  sp2.unlock();

  {
    std::lock_guard<rw_spinlock> lock(sp);
    BOOST_TEST(!sp.try_lock());
    std::lock_guard<rw_spinlock> lock2(sp2);
    BOOST_TEST(!sp.try_lock());
    BOOST_TEST(!sp2.try_lock());
  }
}

void f(rw_spinlock& sp, int n)
{
  for (int i = 0; i < n; ++i) {
    std::lock_guard<rw_spinlock> lock(sp);
    ++count;
  }
}

UNORDERED_AUTO_TEST (rw_spinlock_test3) {
  count = 0;

  rw_spinlock sp;

  int const N = 1000000; // iterations
  int const M = 8;       // threads

  std::thread th[M];

  for (int i = 0; i < M; ++i) {
    th[i] = std::thread([=, &sp] { f(sp, N); });
  }

  for (int i = 0; i < M; ++i) {
    th[i].join();
  }

  BOOST_TEST_EQ(count, N * M);
}

UNORDERED_AUTO_TEST (rw_spinlock_test4) {
  rw_spinlock sp, sp2;

  sp.lock();
  sp2.lock_shared();
  sp2.lock_shared();

  sp.unlock();
  sp2.unlock_shared();
  sp2.unlock_shared();

  {
    std::lock_guard<rw_spinlock> lock(sp);
    boost::shared_lock<rw_spinlock> lock2(sp2);
    boost::shared_lock<rw_spinlock> lock3(sp2);
  }
}

UNORDERED_AUTO_TEST (rw_spinlock_test5) {
  rw_spinlock sp;

  {
    BOOST_TEST(sp.try_lock_shared());
    BOOST_TEST(sp.try_lock_shared());
    sp.unlock_shared();
    sp.unlock_shared();
  }

  {
    BOOST_TEST(sp.try_lock());
    BOOST_TEST(!sp.try_lock_shared());
    sp.unlock();
  }

  {
    std::lock_guard<rw_spinlock> lock(sp);
    BOOST_TEST(!sp.try_lock_shared());
  }

  {
    boost::shared_lock<rw_spinlock> lock(sp);
    BOOST_TEST(!sp.try_lock());
    BOOST_TEST(sp.try_lock_shared());
    sp.unlock_shared();
  }
}

UNORDERED_AUTO_TEST (rw_spinlock_test6) {
  count = 0;

  rw_spinlock sp;

  int const N = 1000000; // total iterations
  int const M = 8;       // threads

  std::thread th[M];

  for (int i = 0; i < M; ++i) {
    int n = N;
    th[i] = std::thread([n, &sp] {
      for (;;) {
        {
          boost::shared_lock<rw_spinlock> lock(sp);
          if (count >= n)
            break;
        }

        {
          std::lock_guard<rw_spinlock> lock(sp);
          if (count >= n)
            break;
          ++count;
        }
      }
    });
  }

  for (int i = 0; i < M; ++i) {
    th[i].join();
  }

  BOOST_TEST_EQ(count, N);
}

UNORDERED_AUTO_TEST (rw_spinlock_test7) {
  rw_spinlock sp;

  int const N = 1000000; // total iterations
  int const M = 8;       // threads

  std::thread th[M];

  for (int i = 0; i < M; ++i) {
    int n = N;
    th[i] = std::thread([=, &sp] {
      for (;;) {
        int oldc;

        {
          boost::shared_lock<rw_spinlock> lock(sp);
          if (count >= n)
            break;
          oldc = count;
        }

        {
          std::lock_guard<rw_spinlock> lock(sp);
          if (count == oldc)
            ++count;
        }
      }
    });
  }

  for (int i = 0; i < M; ++i) {
    th[i].join();
  }

  BOOST_TEST_EQ(count, N);
}

UNORDERED_AUTO_TEST (rw_spinlock_test8) {
  count = 0;

  rw_spinlock sp;

  int const N = 1000; // total iterations
  int const M = 4;    // threads

  std::thread th[M];

  for (int i = 0; i < M; ++i) {
    int k = i;
    int m = M;
    int n = N;

    th[i] = std::thread([k, m, n, &sp] {
      for (int j = 0; j < n; ++j) {
        int oldc;

        for (;;) {
          {
            boost::shared_lock<rw_spinlock> lock(sp);
            oldc = count;
          }

          if (oldc % m == k)
            break;
        }

        {
          std::lock_guard<rw_spinlock> lock(sp);
          if (count == oldc)
            ++count;
        }
      }
    });
  }

  for (int i = 0; i < M; ++i) {
    th[i].join();
  }

  BOOST_TEST_EQ(count, N * M);
}

RUN_TESTS()
