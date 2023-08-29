
// Copyright (C) 2008-2016 Daniel James.
// Copyright (C) 2022 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNORDERED_FWD_HPP_INCLUDED
#define BOOST_UNORDERED_FWD_HPP_INCLUDED

#include <boost/config.hpp>
#if defined(BOOST_HAS_PRAGMA_ONCE)
#pragma once
#endif

#include <boost/predef.h>

#include <utility>

// BOOST_UNORDERED_EMPLACE_LIMIT = The maximum number of parameters in
// emplace (not including things like hints). Don't set it to a lower value, as
// that might break something.

#if !defined BOOST_UNORDERED_EMPLACE_LIMIT
#define BOOST_UNORDERED_EMPLACE_LIMIT 10
#endif

////////////////////////////////////////////////////////////////////////////////
// Configuration
//
// Unless documented elsewhere these configuration macros should be considered
// an implementation detail, I'll try not to break them, but you never know.

// Use Sun C++ workarounds
// I'm not sure which versions of the compiler require these workarounds, so
// I'm just using them of everything older than the current test compilers
// (as of May 2017).

#if !defined(BOOST_UNORDERED_SUN_WORKAROUNDS1)
#if BOOST_COMP_SUNPRO && BOOST_COMP_SUNPRO < BOOST_VERSION_NUMBER(5, 20, 0)
#define BOOST_UNORDERED_SUN_WORKAROUNDS1 1
#else
#define BOOST_UNORDERED_SUN_WORKAROUNDS1 0
#endif
#endif

// BOOST_UNORDERED_TUPLE_ARGS
//
// Maximum number of std::tuple members to support, or 0 if std::tuple
// isn't avaiable. More are supported when full C++11 is used.

// Already defined, so do nothing
#if defined(BOOST_UNORDERED_TUPLE_ARGS)

// Assume if we have C++11 tuple it's properly variadic,
// and just use a max number of 10 arguments.
#elif !defined(BOOST_NO_CXX11_HDR_TUPLE)
#define BOOST_UNORDERED_TUPLE_ARGS 10

// Visual C++ has a decent enough tuple for piecewise construction,
// so use that if available, using _VARIADIC_MAX for the maximum
// number of parameters. Note that this comes after the check
// for a full C++11 tuple.
#elif defined(BOOST_MSVC)
#if !BOOST_UNORDERED_HAVE_PIECEWISE_CONSTRUCT
#define BOOST_UNORDERED_TUPLE_ARGS 0
#elif defined(_VARIADIC_MAX)
#define BOOST_UNORDERED_TUPLE_ARGS _VARIADIC_MAX
#else
#define BOOST_UNORDERED_TUPLE_ARGS 5
#endif

// Assume that we don't have std::tuple
#else
#define BOOST_UNORDERED_TUPLE_ARGS 0
#endif

#if BOOST_UNORDERED_TUPLE_ARGS
#include <tuple>
#endif

#endif
