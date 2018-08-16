// Copyright (C) 2017-2018 ChaosForge Ltd
// http://chaosforge.org/
//
// This file is part of Nova libraries. 
// For conditions of distribution and use, see copying.txt file in root folder.
// 
// This implementation of meta library uses Tacit Style approach developed by Odin Holmes.
// Original implementation https://github.com/odinthenerd/tmp

#pragma once

namespace nv
{
    namespace meta
    {
        template < typename T > void test();

        template < int I > struct int_type
        {
            constexpr static int value = I;
        };

        // clang-format off
        template < typename... Ts > struct type_list {};
        struct identity {};
        template < typename C = identity > struct listify {};
        template < typename R, typename C = listify<> > struct replace {};
        template < typename F, typename C = listify<> > 
        struct foreach {};
        template < typename T, typename C = identity > struct always {};
        template < typename C > struct unpack {};
        template < template < typename... > class F, typename C = identity >
        struct reduce {};
        template < typename F = listify<>, typename C = listify<> >
        struct zip_with_index {};
        // clang-format on
        template < typename C = listify<> > struct reverse
        {
        };

        struct null_type
        {
        };

        /**
         * Example of the reduce functor.
         * It can use a little bit of sfineae in order to catch the first call
         */
        template < typename LHS, typename RHS > struct sum
        {
            using value_type = int_type< LHS::value + RHS::value >;
        };

        /**
         * Implementation of the sum for the first call, just ignore null type
         * on the lhs.
         */
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
            template < template < typename... > class F, typename C >
            struct dispatch< 1, promote< F, C > >
            {
                template < typename T >
                using f = typename dispatch< 1, C >::template f< F< T > >;
            };

            template < template < typename... > class F, typename C >
            struct dispatch< 2, promote< F, C > >
            {
                template < typename T0, typename T1 >
                using f = typename dispatch< 1, C >::template f< F< T0, T1 > >;
            };

            template < template < typename... > class F, typename C >
            struct dispatch< 3, promote< F, C > >
            {
                template < typename T0, typename T1, typename T2 >
                using f =
                    typename dispatch< 1, C >::template f< F< T0, T1, T2 > >;
            };

            template < template < typename... > class F, typename C >
            struct dispatch< 4, promote< F, C > >
            {
                template < typename T0, typename T1, typename T2, typename T3 >
                using f =
                    typename dispatch< 1,
                                       C >::template f< F< T0, T1, T2, T3 > >;
            };

            template < int N, template < typename... > class F, typename C >
            struct dispatch< N, promote< F, C > >
            {
                template < typename... Ts >
                using f = typename dispatch< 1, C >::template f< F< Ts... > >;
            };
        }; // namespace detail

        template < int... Is > struct index_sequence
        {
        };

        namespace detail
        {
            template < int N, int... Is >
            constexpr auto
            push_back_index_sequence( int&&, index_sequence< Is... > && )
                -> index_sequence< Is..., N >;

            template < int S, int C > struct make_index_sequence_impl
            {
                template < typename T >
                using value_type =
                    typename make_index_sequence_impl< S, C + 1 >::
                        template value_type< decltype(
                            push_back_index_sequence< C >( C, T{} ) ) >;
            };

            template < int S > struct make_index_sequence_impl< S, S >
            {
                template < typename T > using value_type = T;
            };
        }; // namespace detail

        template < int S >
        using make_index_sequence =
            typename detail::make_index_sequence_impl< S, 0 >::
                template value_type< index_sequence<> >;

        namespace detail
        {
            template < int N, typename T = make_index_sequence< N > >
            struct zip_with_index_impl;

            template < int N, int... Is >
            struct zip_with_index_impl< N, index_sequence< Is... > >
            {
                template < typename F,
                           template < typename... > class C,
                           typename... Ts >
                using f =
                    C< typename dispatch< 2, F >::template f< int_type< Is >,
                                                              Ts >... >;
            };

            template < int N, typename F, typename C >
            struct dispatch< N, zip_with_index< F, C > >
            {
                template < typename... Ts >
                using f =
                    typename zip_with_index_impl< sizeof...( Ts ) >::template f<
                        F,
                        dispatch< sizeof...( Ts ), C >::template f,
                        Ts... >;
            };
        } // namespace detail

        namespace detail
        {
            template < typename... Ts > constexpr int find_size()
            {
                constexpr int len = sizeof...( Ts );

                if ( len > 3 )
                {
                    return 4;
                }

                return len;
            }

            template < typename C, typename... Ts > struct reverse_impl
            {
                template < typename... Us >
                using f = typename dispatch< sizeof...( Ts ) + sizeof...( Us ),
                                             C >::template f< Ts..., Us... >;
            };

            template < typename C > struct dispatch< 1, reverse< C > >
            {
                template < typename T0 >
                using f = typename dispatch< 1, C >::template f< T0 >;
            };

            template < typename C > struct dispatch< 2, reverse< C > >
            {
                template < typename T0, typename T1 >
                using f = typename dispatch< 2, C >::template f< T1, T0 >;
            };

            template < typename C > struct dispatch< 3, reverse< C > >
            {
                template < typename T0, typename T1, typename T2 >
                using f = typename dispatch< 3, C >::template f< T2, T1, T0 >;
            };

            template < typename C > struct dispatch< 4, reverse< C > >
            {
                template < typename T0,
                           typename T1,
                           typename T2,
                           typename... Ts >
                using f =
                    typename dispatch< find_size< Ts... >() + 1,
                                       reverse< promote< reverse_impl > > >::
                        template f< Ts..., C >::template f< T2, T1, T0 >;
            };

            template < int N, typename C > struct dispatch< N, reverse< C > >
            {
                template < typename... Ts >
                using f =
                    typename dispatch< find_size< Ts... >(),
                                       reverse< C > >::template f< Ts... >;
            };
        } // namespace detail


        namespace detail
        {
            template < int N, typename F, typename C >
            struct dispatch< N, foreach< F, C > >
            {
                template < typename... Ts >
                using f = typename dispatch< sizeof...( Ts ), C >::template f<
                    typename dispatch< 1, F >::template f< Ts >... >;
            };
        } // namespace detail

        template < int N, typename T, typename C = listify<> >
        struct gen_n_types
        {
        };

        namespace detail
        {
            template < typename C, typename... Ts > struct gen_n_types_impl
            {
                template < typename... Vs >
                using f = typename dispatch< sizeof...( Ts ) + sizeof...( Vs ),
                                             C >::template f< Ts..., Vs... >;
            };

            template < int N, int I, typename T, typename C >
            struct dispatch< N, gen_n_types< I, T, C > >;


            template < int N, typename T, typename C >
            struct dispatch< N, gen_n_types< 1, T, C > >
            {
                template < typename... Ts >
                using f = typename dispatch< sizeof...( Ts ) + 1,
                                             C >::template f< Ts..., T >;
            };

            template < int N, typename T, typename C >
            struct dispatch< N, gen_n_types< 2, T, C > >
            {
                template < typename... Ts >
                using f = typename dispatch< sizeof...( Ts ) + 2,
                                             C >::template f< Ts..., T, T >;
            };

            template < int N, typename T, typename C >
            struct dispatch< N, gen_n_types< 3, T, C > >
            {
                template < typename... Ts >
                using f = typename dispatch< sizeof...( Ts ) + 3,
                                             C >::template f< Ts..., T, T, T >;
            };

            template < int N, int I, typename T, typename C >
            struct dispatch< N, gen_n_types< I, T, C > >
            {
                template < typename... Ts >
                using f = typename dispatch<
                    sizeof...( Ts ) + 1,
                    gen_n_types< I - 3, T, promote< gen_n_types_impl > > >::
                    template f< C, Ts... >::template f< T, T, T >;
            };

        } // namespace detail

        /** call the meta functor */
        template < typename F, typename... Ts >
        using call = typename detail::dispatch< sizeof...( Ts ),
                                                F >::template f< Ts... >;
    } // namespace meta
} // namespace nv


namespace nv
{
    namespace meta
    {
        template < bool > struct conditional
        {
            template < typename F, typename T > using value_type = F;
        };

        template <> struct conditional< true >
        {
            template < typename F, typename T > using value_type = T;
        };

        struct false_type
        {
            constexpr static bool value = false;
        };

        struct true_type
        {
            constexpr static bool value = true;
        };

        namespace detail
        {
            template < typename T0, typename T1 > struct is_same_impl
            {
                using value_type = false_type;
            };

            template < typename T > struct is_same_impl< T, T >
            {
                using value_type = true_type;
            };
        } // namespace detail

        template < typename T0, typename T1 >
        using is_same = typename detail::is_same_impl< T0, T1 >::value_type;
    } // namespace meta
} // namespace nv

