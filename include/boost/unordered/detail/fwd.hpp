
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

#include <tuple>

#endif
