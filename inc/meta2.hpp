#pragma once

namespace nv
{
    namespace meta
    {
        template < int I > struct int_type
        {
            constexpr static int value = I;
        };


        // clang-format off
        template < typename... Ts > struct type_list {};
        struct identity {};
        template < typename C = identity > struct listify {};
        template < typename R, typename C = listify<> > struct replace {};
        template < typename T, typename C = identity > struct always {};
        template < typename C > struct unpack {};
        // clang-format on
        template < template < typename... > class F, typename C = identity >
        struct reduce
        {
        };

        struct null_type
        {
        };


        template < typename LHS, typename RHS > struct sum
        {
            using value_type = int_type< LHS::value + RHS::value >;
        };

        template < typename RHS > struct sum< null_type, RHS >
        {
            using value_type = RHS;
        };

        namespace detail
        {
            template < int N, typename C > struct dispatch;

            template <> struct dispatch< 1, identity >
            {
                template < typename T > using f = T;
            };

            template < int N, typename T, typename C >
            struct dispatch< N, always< T, C > >
            {
                template < typename... >
                using f = typename dispatch< 1, C >::template f< T >;
            };

            template < int N, typename C > struct dispatch< N, listify< C > >
            {
                template < typename... Ts >
                using f =
                    type_list< typename dispatch< 1, C >::template f< Ts >... >;
            };

            template < int N, typename T, typename C >
            struct dispatch< N, replace< T, C > >
            {
                template < typename... Ts >
                using f = typename dispatch< sizeof...( Ts ), C >::template f<
                    typename dispatch< 1, always< T > >::template f< Ts >... >;
            };

            template < typename T, typename C >
            struct dispatch< 1, replace< T, C > >
            {
                template < typename >
                using f = typename dispatch< 1, C >::template f< T >;
            };

            template < typename C, typename L > struct unpack_impl;

            template < typename C,
                       template < typename... > class L,
                       typename... Ls >
            struct unpack_impl< C, L< Ls... > >
            {
                using value_type = typename dispatch< sizeof...( Ls ),
                                                      C >::template f< Ls... >;
            };

            template < int N, typename C > struct dispatch< N, unpack< C > >
            {
                template < typename L >
                using f = typename unpack_impl< C, L >::value_type;
            };

            template < int N,
                       template < typename... > class F,
                       typename I,
                       typename... Ts >
            struct reduce_impl;

            // using the SFINEA for walking through the elements of the list
            // probably there is a better way of doing this since the SFINAE
            // is the slowest template mechanism. For now it's ok.
            // @TODO fix it!
            template < int N,
                       template < typename... > class F,
                       typename I,
                       typename H,
                       typename... Ts >
            struct reduce_impl< N, F, I, H, Ts... >
            {
                template < typename R >
                using value_type = typename reduce_impl< N - 1, F, I, Ts... >::
                    template value_type< typename F< R, H >::value_type >;
            };

            template < template < typename... > class F,
                       typename I,
                       typename... Ts >
            struct reduce_impl< 0, F, I, Ts... >
            {
                template < typename R > using value_type = R;
            };

            template < int N, template < typename... > class F, typename C >
            struct dispatch< N, reduce< F, C > >
            {
                template < typename... Ts >
                using f =
                    typename dispatch< 1, C >::template f< typename reduce_impl<
                        sizeof...( Ts ),
                        F,
                        null_type,
                        Ts... >::template value_type< null_type > >;
            };
        } // namespace detail

        template < template < typename... > class F, typename C = identity >
        struct promote
        {
        };

        namespace detail
        {
            template < int N, template < typename... > class F, typename C >
            struct dispatch< N, promote< F, C > >
            {
                template < typename... T >
                using f = typename dispatch< 1, C >::template f< F< T >... >;
            };
        }; // namespace detail

        /** call the meta functor */
        template < typename F, typename... Ts >
        using call = typename detail::dispatch< sizeof...( Ts ),
                                                F >::template f< Ts... >;
    } // namespace meta
} // namespace nv

template < typename T >
using calc_sizeof = nv::meta::int_type< sizeof( T ) >;

using test_replace = nv::meta::replace< int >;
using data         = nv::meta::call< nv::meta::unpack< test_replace >,
                             nv::meta::type_list< int, float, char > >;

using test_reduce =
    nv::meta::promote< calc_sizeof, nv::meta::reduce< nv::meta::sum > >;
using test_reduce_data = nv::meta::call< test_reduce, float, int, char, bool >;
