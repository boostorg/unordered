/* Hash function characterization.
 *
 * Copyright 2022-2025 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_HASH_TRAITS_HPP
#define BOOST_UNORDERED_HASH_TRAITS_HPP

#include <boost/config/pragma_message.hpp>
#include <boost/container_hash/hash_is_avalanching.hpp>

#if !defined(BOOST_ALLOW_DEPRECATED_HEADERS)&&!defined(BOOST_ALLOW_DEPRECATED)
BOOST_PRAGMA_MESSAGE("Deprecated boost::unordered::hash_is_avalanching is now "
"a using-declaration of boost::hash_is_avalanching in "
"<boost/container_hash/hash_is_avalanching.hpp>. "
"Use that header directly instead.")
#endif

namespace boost{
namespace unordered{

using boost::hash_is_avalanching;

} /* namespace unordered */
} /* namespace boost */

#endif
