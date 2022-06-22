// Copyright (C) 2022 Joaquin M Lopez Munoz.
// Copyright (C) 2022 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNORDERED_DETAIL_PRIME_FMOD_HPP
#define BOOST_UNORDERED_DETAIL_PRIME_FMOD_HPP

#include <boost/config.hpp>
#if defined(BOOST_HAS_PRAGMA_ONCE)
#pragma once
#endif

#include <boost/cstdint.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/transform.hpp>

#include <climits>

namespace boost {
  namespace unordered {
    namespace detail {

#if defined(SIZE_MAX)
#if ((((SIZE_MAX >> 16) >> 16) >> 16) >> 15) != 0
#define BOOST_UNORDERED_FCA_HAS_64B_SIZE_T
#endif
#elif defined(UINTPTR_MAX) /* used as proxy for std::size_t */
#if ((((UINTPTR_MAX >> 16) >> 16) >> 16) >> 15) != 0
#define BOOST_UNORDERED_FCA_HAS_64B_SIZE_T
#endif
#endif

#if !defined(BOOST_NO_INT64_T) &&                                              \
  (defined(BOOST_HAS_INT128) || (defined(BOOST_MSVC) && defined(_M_X64)))
#define BOOST_UNORDERED_FCA_FASTMOD_SUPPORT
#endif

// clang-format off
#define BOOST_UNORDERED_PRIME_FMOD_SIZES_32BIT_INCOMPLETE                      \
  (13ul)                                                                       \
  (29ul)                                                                       \
  (53ul)                                                                       \
  (97ul)                                                                       \
  (193ul)                                                                      \
  (389ul)                                                                      \
  (769ul)                                                                      \
  (1543ul)                                                                     \
  (3079ul)                                                                     \
  (6151ul)                                                                     \
  (12289ul)                                                                    \
  (24593ul)                                                                    \
  (49157ul)                                                                    \
  (98317ul)                                                                    \
  (196613ul)                                                                   \
  (393241ul)                                                                   \
  (786433ul)                                                                   \
  (1572869ul)                                                                  \
  (3145739ul)                                                                  \
  (6291469ul)                                                                  \
  (12582917ul)                                                                 \
  (25165843ul)                                                                 \
  (50331653ul)                                                                 \
  (100663319ul)                                                                \
  (201326611ul)                                                                \
  (402653189ul)                                                                \
  (805306457ul)                                                                \
  (1610612741ul)                                                               \
  (3221225473ul)
      // clang-format on

#if !defined(BOOST_UNORDERED_FCA_HAS_64B_SIZE_T)

// we attempt to double the previous size in our prime number sequence but
// unfortunately for 32 bit platforms, the closest prime to 2 * 3221225473ul
// would exceed numeric limits so we instead settle for the prime closest to
// 2^32 - 1
//
#define BOOST_UNORDERED_PRIME_FMOD_SIZES_32BIT                                 \
  BOOST_UNORDERED_PRIME_FMOD_SIZES_32BIT_INCOMPLETE(4294967291ul)

#define BOOST_UNORDERED_PRIME_FMOD_SIZES_64BIT

#else

// for 64 bit platforms, we consider our 32 bit prime number sequence to be
// complete
//
#define BOOST_UNORDERED_PRIME_FMOD_SIZES_32BIT                                 \
  BOOST_UNORDERED_PRIME_FMOD_SIZES_32BIT_INCOMPLETE

      // for 64 bit platforms, the remaining prime numbers should be:
      // (6442450939ul)
      // (12884901893ul)
      // (25769803751ul)
      // (51539607551ul)
      // (103079215111ul)
      // (206158430209ul)
      // (412316860441ul)
      // (824633720831ul)
      // (1649267441651ul)
      //
      // but this causes problems on versions of mingw where the `long` type is
      // 32 bits, even for 64-bit targets. We work around this by replacing the
      // literals with compile-time arithmetic, using bitshifts to reconstruct
      // the number. The math can be verified using the code below:
      //
      // (1 << 32) + 2147483643 == 6442450939
      // (3 << 32) + 5 == 12884901893
      // (5 << 32) + 4294967271 == 25769803751
      // (11 << 32) + 4294967295 == 51539607551
      // (24 << 32) + 7 == 103079215111
      // (48 << 32) + 1 == 206158430209
      // (96 << 32) + 25 == 412316860441
      // (191 << 32) + 4294967295 == 824633720831
      // (383 << 32) + 4294967283 == 1649267441651
      //

// clang-format off
#define BOOST_UNORDERED_PRIME_FMOD_SIZES_64BIT                                   \
  ((boost::ulong_long_type(1ul) << 32)   + boost::ulong_long_type(2147483643ul)) \
  ((boost::ulong_long_type(3ul) << 32)   + boost::ulong_long_type(5ul))          \
  ((boost::ulong_long_type(5ul) << 32)   + boost::ulong_long_type(4294967271ul)) \
  ((boost::ulong_long_type(11ul) << 32)  + boost::ulong_long_type(4294967295ul)) \
  ((boost::ulong_long_type(24ul) << 32)  + boost::ulong_long_type(7ul))          \
  ((boost::ulong_long_type(48ul) << 32)  + boost::ulong_long_type(1ul))          \
  ((boost::ulong_long_type(96ul) << 32)  + boost::ulong_long_type(25ul))         \
  ((boost::ulong_long_type(191ul) << 32) + boost::ulong_long_type(4294967295ul)) \
  ((boost::ulong_long_type(383ul) << 32) + boost::ulong_long_type(4294967283ul))
      // clang-format on

#endif /* BOOST_UNORDERED_FCA_HAS_64B_SIZE_T */

#define BOOST_UNORDERED_PRIME_FMOD_SIZES                                       \
  BOOST_UNORDERED_PRIME_FMOD_SIZES_32BIT BOOST_UNORDERED_PRIME_FMOD_SIZES_64BIT

#if defined(BOOST_UNORDERED_FCA_FASTMOD_SUPPORT)

#define BOOST_UNORDERED_FCA_INV(r, data, elem) (UINT64_MAX / elem + 1)

#define BOOST_UNORDERED_PRIME_FMOD_INV_SIZES32                                 \
  BOOST_PP_SEQ_TRANSFORM(                                                      \
    BOOST_UNORDERED_FCA_INV, 0, BOOST_UNORDERED_PRIME_FMOD_SIZES_32BIT)

#endif /* BOOST_UNORDERED_FCA_FASTMOD_SUPPORT */

      // Because we compile for C++03, we don't have access to any inline
      // initialization for array data members so the definitions must exist
      // out-of-line. To keep the library header-only, we introduce a dummy
      // template parameter which permits the definition to be included in
      // multiple TUs without conflict.
      //
      template <class = void> struct prime_fmod_size
      {
        static std::size_t sizes[];
        static std::size_t const sizes_len;
        static std::size_t (*positions[])(std::size_t);

#if defined(BOOST_UNORDERED_FCA_FASTMOD_SUPPORT)
        // these are the reciprocals required for the fast_modulo routines, i.e.
        // the result of `UINT64_C (0 xFFFFFFFFFFFFFFFF ) / d + 1;` from the
        // Lemire paper
        //
        static boost::uint64_t inv_sizes32[];
        static std::size_t const inv_sizes32_len;
#endif

        static inline std::size_t size_index(std::size_t n)
        {
          std::size_t i = 0;
          for (; i < (sizes_len - 1); ++i) {
            if (sizes[i] >= n) {
              break;
            }
          }
          return i;
        }

        static inline std::size_t size(std::size_t size_index)
        {
          return sizes[size_index];
        }

        template <std::size_t Size> static std::size_t modulo(std::size_t hash)
        {
          return hash % Size;
        }

#if defined(BOOST_UNORDERED_FCA_FASTMOD_SUPPORT)
        // We emulate the techniques taken from:
        // Faster Remainder by Direct Computation: Applications to Compilers and
        // Software Libraries
        // https://arxiv.org/abs/1902.01961
        //
        // In essence, use fancy math to directly calculate the remainder (aka
        // modulo) exploiting how compilers transform division
        //

#if defined(BOOST_MSVC)
        static inline uint64_t get_remainder(uint64_t fractional, uint32_t d)
        {
          // use msvc instrinsic when available instead of using `>> 64`
          //
          return __umulh(fractional, d);
        }
#else
        static inline uint64_t get_remainder(uint64_t fractional, uint32_t d)
        {
          return static_cast<uint64_t>(
            ((boost::uint128_type)fractional * d) >> 64);
        }
#endif /* defined(BOOST_MSVC) */

        static inline uint32_t fast_modulo(uint32_t a, uint64_t M, uint32_t d)
        {
          uint64_t fractional = M * a;
          return (uint32_t)(get_remainder(fractional, d));
        }
#endif /* defined(BOOST_UNORDERED_FCA_FASTMOD_SUPPORT) */

        static inline std::size_t position(
          std::size_t hash, std::size_t size_index)
        {
#if defined(BOOST_UNORDERED_FCA_FASTMOD_SUPPORT)
#if defined(BOOST_UNORDERED_FCA_HAS_64B_SIZE_T)
          if (BOOST_LIKELY(size_index < inv_sizes32_len)) {
            return fast_modulo(uint32_t(hash) + uint32_t(hash >> 32),
              inv_sizes32[size_index], uint32_t(sizes[size_index]));
          } else {
            return positions[size_index - inv_sizes32_len](hash);
          }
#else
          return fast_modulo(
            hash, inv_sizes32[size_index], uint32_t(sizes[size_index]));
#endif /* defined(BOOST_UNORDERED_FCA_HAS_64B_SIZE_T) */
#else
          return positions[size_index](hash);
#endif /* defined(BOOST_UNORDERED_FCA_FASTMOD_SUPPORT) */
        }
      };

      template <class T>
      std::size_t prime_fmod_size<T>::sizes[] = {
        BOOST_PP_SEQ_ENUM(BOOST_UNORDERED_PRIME_FMOD_SIZES)};

      template <class T>
      std::size_t const prime_fmod_size<T>::sizes_len = BOOST_PP_SEQ_SIZE(
        BOOST_UNORDERED_PRIME_FMOD_SIZES);

#if defined(BOOST_UNORDERED_FCA_FASTMOD_SUPPORT)
      template <class T>
      std::size_t prime_fmod_size<T>::inv_sizes32[] = {
        BOOST_PP_SEQ_ENUM(BOOST_UNORDERED_PRIME_FMOD_INV_SIZES32)};

      template <class T>
      std::size_t const prime_fmod_size<T>::inv_sizes32_len = BOOST_PP_SEQ_SIZE(
        BOOST_UNORDERED_PRIME_FMOD_INV_SIZES32);
#endif

#define BOOST_UNORDERED_PRIME_FMOD_POSITIONS_ELEMENT(z, _, n)                  \
  prime_fmod_size<T>::template modulo<n>,

      template <class T>
      std::size_t (*prime_fmod_size<T>::positions[])(std::size_t) = {
#if !defined(BOOST_UNORDERED_FCA_FASTMOD_SUPPORT)
        BOOST_PP_SEQ_FOR_EACH(BOOST_UNORDERED_PRIME_FMOD_POSITIONS_ELEMENT, ~,
          BOOST_UNORDERED_PRIME_FMOD_SIZES_32BIT)
#endif

#if defined(BOOST_UNORDERED_FCA_HAS_64B_SIZE_T)
          BOOST_PP_SEQ_FOR_EACH(BOOST_UNORDERED_PRIME_FMOD_POSITIONS_ELEMENT, ~,
            BOOST_UNORDERED_PRIME_FMOD_SIZES_64BIT)
#endif
      };

#undef BOOST_UNORDERED_PRIME_FMOD_POSITIONS_ELEMENT
#undef BOOST_UNORDERED_PRIME_FMOD_SIZES
#undef BOOST_UNORDERED_PRIME_FMOD_SIZES_64BIT
#undef BOOST_UNORDERED_PRIME_FMOD_SIZES_32BIT
#undef BOOST_UNORDERED_PRIME_FMOD_SIZES_32BIT_INCOMPLETE

    } // namespace detail
  }   // namespace unordered
} // namespace boost

#endif // BOOST_UNORDERED_DETAIL_PRIME_FMOD_HPP
