# Copyright 2019 Sam Day
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
#
# NOTE: CMake support for Boost.unordered is currently experimental at best
#       and the interface is likely to change in the future

cmake_minimum_required(VERSION 3.5)
project(BoostUnordered LANGUAGES CXX)

add_library(boost_unordered INTERFACE)
add_library(Boost::unordered ALIAS boost_unordered)

target_include_directories(boost_unordered INTERFACE include)

target_link_libraries(boost_unordered
        INTERFACE
        Boost::assert
        Boost::config
        Boost::container
        Boost::core
        Boost::detail
        Boost::functional
        Boost::move
        Boost::predef
        Boost::preprocessor
        Boost::smart_ptr
        Boost::throw_exception
        Boost::tuple
        Boost::type_traits
        Boost::utility
        )
