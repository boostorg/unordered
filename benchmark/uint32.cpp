// Copyright 2021 Peter Dimov.
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING

#include <boost/unordered_map.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/core/detail/splitmix64.hpp>
#ifdef HAVE_ABSEIL
# include "absl/container/node_hash_map.h"
# include "absl/container/flat_hash_map.h"
#endif
#include <unordered_map>
#include <map>
#include <cstdint>
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

static void print_time( std::chrono::steady_clock::time_point & t1, char const* label, std::uint32_t s, std::size_t size )
{
    auto t2 = std::chrono::steady_clock::now();

    std::cout << label << ": " << ( t2 - t1 ) / 1ms << " ms (s=" << s << ", size=" << size << ")\n";

    t1 = t2;
}

constexpr unsigned N = 2'000'000;
constexpr int K = 10;

static std::vector< std::uint32_t > indices1, indices2, indices3;

static void init_indices()
{
    indices1.push_back( 0 );

    for( unsigned i = 1; i <= N*2; ++i )
    {
        indices1.push_back( i );
    }

    indices2.push_back( 0 );

    {
        boost::detail::splitmix64 rng;

        for( unsigned i = 1; i <= N*2; ++i )
        {
            indices2.push_back( static_cast<std::uint32_t>( rng() ) );
        }
    }

    indices3.push_back( 0 );

    for( unsigned i = 1; i <= N*2; ++i )
    {
        indices3.push_back( (std::uint32_t)i << 11 );
    }
}

template<class Map> void test_insert( Map& map, std::chrono::steady_clock::time_point & t1 )
{
    for( unsigned i = 1; i <= N; ++i )
    {
        map.insert( { indices1[ i ], i } );
    }

    print_time( t1, "Consecutive insert",  0, map.size() );

    for( unsigned i = 1; i <= N; ++i )
    {
        map.insert( { indices2[ i ], i } );
    }

    print_time( t1, "Random insert",  0, map.size() );

    for( unsigned i = 1; i <= N; ++i )
    {
        map.insert( { indices3[ i ], i } );
    }

    print_time( t1, "Consecutive shifted insert",  0, map.size() );

    std::cout << std::endl;
}

template<class Map> void test_lookup( Map& map, std::chrono::steady_clock::time_point & t1 )
{
    std::uint32_t s;
    
    s = 0;

    for( int j = 0; j < K; ++j )
    {
        for( unsigned i = 1; i <= N * 2; ++i )
        {
            auto it = map.find( indices1[ i ] );
            if( it != map.end() ) s += it->second;
        }
    }

    print_time( t1, "Consecutive lookup",  s, map.size() );

    s = 0;

    for( int j = 0; j < K; ++j )
    {
        for( unsigned i = 1; i <= N * 2; ++i )
        {
            auto it = map.find( indices2[ i ] );
            if( it != map.end() ) s += it->second;
        }
    }

    print_time( t1, "Random lookup",  s, map.size() );

    s = 0;

    for( int j = 0; j < K; ++j )
    {
        for( unsigned i = 1; i <= N * 2; ++i )
        {
            auto it = map.find( indices3[ i ] );
            if( it != map.end() ) s += it->second;
        }
    }

    print_time( t1, "Consecutive shifted lookup",  s, map.size() );

    std::cout << std::endl;
}

template<class Map> void test_iteration( Map& map, std::chrono::steady_clock::time_point & t1 )
{
    auto it = map.begin();

    while( it != map.end() )
    {
        if( it->second & 1 )
        {
            map.erase( it++ );
        }
        else
        {
            ++it;
        }
    }

    print_time( t1, "Iterate and erase odd elements",  0, map.size() );

    std::cout << std::endl;
}

template<class Map> void test_erase( Map& map, std::chrono::steady_clock::time_point & t1 )
{
    for( unsigned i = 1; i <= N; ++i )
    {
        map.erase( indices1[ i ] );
    }

    print_time( t1, "Consecutive erase",  0, map.size() );

    {
        boost::detail::splitmix64 rng;

        for( unsigned i = 1; i <= N; ++i )
        {
            map.erase( indices2[ i ] );
        }
    }

    print_time( t1, "Random erase",  0, map.size() );

    for( unsigned i = 1; i <= N; ++i )
    {
        map.erase( indices3[ i ] );
    }

    print_time( t1, "Consecutive shifted erase",  0, map.size() );

    std::cout << std::endl;
}

static std::vector< std::pair<std::string, long long> > times;

template<template<class...> class Map> void test( char const* label )
{
    std::cout << label << ":\n\n";

    Map<std::uint32_t, std::uint32_t> map;

    auto t0 = std::chrono::steady_clock::now();
    auto t1 = t0;

    test_insert( map, t1 );
    test_lookup( map, t1 );
    test_iteration( map, t1 );
    test_lookup( map, t1 );
    test_erase( map, t1 );

    auto tN = std::chrono::steady_clock::now();
    std::cout << "Total: " << ( tN - t0 ) / 1ms << " ms\n\n";

    times.push_back( { label, ( tN - t0 ) / 1ms } );
}

// multi_index emulation of unordered_map

template<class K, class V> struct pair
{
    K first;
    mutable V second;
};

using namespace boost::multi_index;

template<class K, class V> using multi_index_map = multi_index_container<
  pair<K, V>,
  indexed_by<
    hashed_unique< member<pair<K, V>, K, &pair<K, V>::first> >
  >
>;

int main()
{
    init_indices();

    test<std::unordered_map>( "std::unordered_map" );
    test<boost::unordered_map>( "boost::unordered_map" );
    test<multi_index_map>( "multi_index_map" );

    // test<std::map>( "std::map" );

#ifdef HAVE_ABSEIL

    test<absl::node_hash_map>( "absl::node_hash_map" );
    test<absl::flat_hash_map>( "absl::flat_hash_map" );

#endif

    std::cout << "---\n\n";

    for( auto const& x: times )
    {
        std::cout << x.first << ": " << x.second << " ms\n";
    }
}

#ifdef HAVE_ABSEIL
# include "absl/container/internal/raw_hash_set.cc"
# include "absl/hash/internal/hash.cc"
# include "absl/hash/internal/low_level_hash.cc"
# include "absl/hash/internal/city.cc"
#endif
