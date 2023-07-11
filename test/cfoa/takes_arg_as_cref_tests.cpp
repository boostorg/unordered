// Copyright 2023 Joaquin M Lopez Munoz
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/config.hpp>
#include <boost/unordered/detail/foa/takes_arg_as_const_reference.hpp>
#include <boost/core/lightweight_test.hpp>

using boost::unordered::detail::foa::takes_arg_as_const_reference;

using  f1  = void ( const int& );
void   function2( const int& ) noexcept;
using  f2  = decltype(function2);
using  f3  = void ( const int&, char* );
using  f4  = void ( const int&, ... );
using  f5  = void ( int& );
void   function6( int& ) noexcept;
using  f6  = decltype(function6);
using  f7  = void ( int&, char* );
using  f8  = void ( int&, ... );
struct f9  { void operator()( const int& ); };
struct f10 { void operator()( const int& ) const; };
struct f11 { void operator()( const int& ) volatile; };
struct f12 { void operator()( const int& ) const volatile; };
struct f13 { void operator()( const int& )&; };
struct f14 { void operator()( const int& ) const&; };
struct f15 { void operator()( const int& ) volatile&; };
struct f16 { void operator()( const int& ) const volatile&; };
struct f17 { void operator()( const int& )&&; };
struct f18 { void operator()( const int& ) const&&; };
struct f19 { void operator()( const int& ) volatile&&; };
struct f20 { void operator()( const int& ) const volatile&&; };
struct f21 { void operator()( const int& ) noexcept; };
struct f22 { void operator()( const int& ) const noexcept; };
struct f23 { void operator()( const int&, int=0 ); };
struct f24 { void operator()( const int&, ... ) noexcept; };
struct f25 { void operator()( int& ); };
struct f26 { void operator()( int& ) const; };
struct f27 { void operator()( int& ) volatile; };
struct f28 { void operator()( int& ) const volatile; };
struct f29 { void operator()( int& )&; };
struct f30 { void operator()( int& ) const&; };
struct f31 { void operator()( int& ) volatile&; };
struct f32 { void operator()(  int& ) const volatile&; };
struct f33 { void operator()( int& )&&; };
struct f34 { void operator()( int& ) const&&; };
struct f35 { void operator()( int& ) volatile&&; };
struct f36 { void operator()( int& ) const volatile&&; };
struct f37 { void operator()( int& ) noexcept; };
struct f38 { void operator()( int& ) const noexcept; };
struct f39 { void operator()( int&, int=0 ); };
struct f40 { void operator()( int&, ... ) noexcept; };
struct f41 { template<typename Arg> void operator()( const Arg& ); };
struct f42 { template<typename Arg> void operator()( const Arg& ) const; };
struct f43 { template<typename Arg> void operator()( const Arg& ) const noexcept; };
struct f44 { template<typename Arg> void operator()( Arg& ); };
struct f45 { template<typename Arg> void operator()( Arg& ) const; };
struct f46 { template<typename Arg> void operator()( Arg& ) const noexcept; };
struct f47 { template<typename Arg> void operator()( Arg ); };
struct f48 { template<typename Arg> void operator()( Arg ) const; };
struct f49 { template<typename Arg> void operator()( Arg ) const noexcept; };
struct f50
{
    void operator()( const int& );
    void operator()( char* );
};
struct f51 // expected false negative
{
    void operator()( const int& );
    template<typename Arg> void operator()( Arg& );
};
using f52=int; // detection doesn't crash even if requirements violated

int main()
{
    auto  lambda53 = []( const int& ){};
    using f53=decltype(lambda53);
    int   retrieved=0;
    auto  lambda54 = [&]( const int& x ) mutable { retrieved = x; };
    using f54=decltype(lambda54);
    auto  lambda55 = []( int& ){};
    using f55=decltype(lambda55);
    auto  lambda56 = [&]( int& x ) mutable { retrieved = x; };
    using f56=decltype(lambda56);

#if !defined(BOOST_NO_CXX14_GENERIC_LAMBDAS)
    auto  lambda57 = []( const auto& ){};
    using f57=decltype(lambda57);
    auto  lambda58 = [&]( const auto& x ) mutable { retrieved = x; };
    using f58=decltype(lambda58);
    auto  lambda59 = []( auto& ){};
    using f59=decltype(lambda59);
    auto  lambda60 = [&]( auto& x ) mutable { retrieved = x; };
    using f60=decltype(lambda60);
#endif

    BOOST_TEST((  takes_arg_as_const_reference<f1,  int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f2,  int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f3,  int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f4,  int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f5,  int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f6,  int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f7,  int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f8,  int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f1*, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f2*, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f3*, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f4*, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f5*, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f6*, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f7*, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f8*, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f1&, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f2&, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f3&, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f4&, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f5&, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f6&, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f7&, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f8&, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f9,  int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f10, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f11, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f12, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f13, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f14, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f15, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f16, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f17, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f18, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f19, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f20, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f21, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f22, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f23, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f24, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f25, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f26, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f27, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f28, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f29, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f30, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f31, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f32, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f33, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f34, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f35, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f36, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f37, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f38, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f39, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f40, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f41, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f42, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f43, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f44, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f45, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f46, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f47, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f48, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f49, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f50, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f51, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f52, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f53, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f54, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f55, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f56, int>::value ));

#if !defined(BOOST_NO_CXX14_GENERIC_LAMBDAS)
    BOOST_TEST((  takes_arg_as_const_reference<f57, int>::value ));
    BOOST_TEST((  takes_arg_as_const_reference<f58, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f59, int>::value ));
    BOOST_TEST(( !takes_arg_as_const_reference<f60, int>::value ));
#endif

    return boost::report_errors();
}
