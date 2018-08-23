// Copyright (C) 2017-2018 ChaosForge Ltd
// http://chaosforge.org/
//
// This file is part of Nova libraries.
// For conditions of distribution and use, see copying.txt file in root folder.
//
// This implementation of meta library uses Tacit Style approach developed by
// Odin Holmes. Original implementation https://github.com/odinthenerd/tmp

#pragma once

#include "logger.hpp"

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
                using f = type_list< typename dispatch< 1, C >::template f< Ts >... >;
            };

            template < int N, typename T, typename C >
            struct dispatch< N, replace< T, C > >
            {
                template < typename... Ts >
                using f = typename dispatch< sizeof...( Ts ), C >::template f<
                    typename dispatch< 1, always< T > >::template f< Ts >... >;
            };

            template < typename T, typename C > struct dispatch< 1, replace< T, C > >
            {
                template < typename >
                using f = typename dispatch< 1, C >::template f< T >;
            };

            template < typename C, typename L > struct unpack_impl;

            template < typename C, template < typename... > class L, typename... Ls >
            struct unpack_impl< C, L< Ls... > >
            {
                using value_type =
                    typename dispatch< sizeof...( Ls ), C >::template f< Ls... >;
            };

            template < int N, typename C > struct dispatch< N, unpack< C > >
            {
                template < typename L >
                using f = typename unpack_impl< C, L >::value_type;
            };

            template < int N, template < typename... > class F, typename I,
                       typename... Ts >
            struct reduce_impl;

            // using the SFINEA for walking through the elements of the list
            // probably there is a better way of doing this since the SFINAE
            // is the slowest template mechanism. For now it's ok.
            // @TODO fix it!
            template < int N, template < typename... > class F, typename I, typename H,
                       typename... Ts >
            struct reduce_impl< N, F, I, H, Ts... >
            {
                template < typename R >
                using value_type =
                    typename reduce_impl< N - 1, F, I, Ts... >::template value_type<
                        typename F< R, H >::value_type >;
            };

            template < template < typename... > class F, typename I, typename... Ts >
            struct reduce_impl< 0, F, I, Ts... >
            {
                template < typename R > using value_type = R;
            };

            template < int N, template < typename... > class F, typename C >
            struct dispatch< N, reduce< F, C > >
            {
                template < typename... Ts >
                using f = typename dispatch< 1, C >::template f<
                    typename reduce_impl< sizeof...( Ts ), F, null_type,
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
            constexpr auto push_back_index_sequence( int&&, index_sequence< Is... > && )
                -> index_sequence< Is..., N >;

            template < int S, int C > struct make_index_sequence_impl
            {
                template < typename T >
                using value_type =
                    typename make_index_sequence_impl< S, C + 1 >::template value_type<
                        decltype( push_back_index_sequence< C >( C, T{} ) ) >;
            };

            template < int S > struct make_index_sequence_impl< S, S >
            {
                template < typename T > using value_type = T;
            };
        }; // namespace detail

        template < int S >
        using make_index_sequence = typename detail::make_index_sequence_impl<
            S, 0 >::template value_type< index_sequence<> >;

        namespace detail
        {
            template < int N, typename T = make_index_sequence< N > >
            struct zip_with_index_impl;

            template < int N, int... Is >
            struct zip_with_index_impl< N, index_sequence< Is... > >
            {
                template < typename F, template < typename... > class C, typename... Ts >
                using f =
                    C< typename dispatch< 2, F >::template f< int_type< Is >, Ts >... >;
            };

            template < int N, typename F, typename C >
            struct dispatch< N, zip_with_index< F, C > >
            {
                template < typename... Ts >
                using f = typename zip_with_index_impl< sizeof...( Ts ) >::template f<
                    F, dispatch< sizeof...( Ts ), C >::template f, Ts... >;
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
                template < typename T0, typename T1, typename T2, typename... Ts >
                using f = typename dispatch< find_size< Ts... >() + 1,
                                             reverse< promote< reverse_impl > > >::
                    template f< Ts..., C >::template f< T2, T1, T0 >;
            };

            template < int N, typename C > struct dispatch< N, reverse< C > >
            {
                template < typename... Ts >
                using f = typename dispatch< find_size< Ts... >(),
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

        template < int N, typename T, typename C = listify<> > struct gen_n_types
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
                using f =
                    typename dispatch< sizeof...( Ts ) + 1, C >::template f< Ts..., T >;
            };

            template < int N, typename T, typename C >
            struct dispatch< N, gen_n_types< 2, T, C > >
            {
                template < typename... Ts >
                using f = typename dispatch< sizeof...( Ts ) + 2, C >::template f< Ts...,
                                                                                   T, T >;
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
        using call = typename detail::dispatch< sizeof...( Ts ), F >::template f< Ts... >;
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
            template < typename T > struct neg_impl;

            template <> struct neg_impl< false_type >
            {
                using value_type = true_type;
            };

            template <> struct neg_impl< true_type >
            {
                using value_type = false_type;
            };
        } // namespace detail

        template < typename T > using neg = typename detail::neg_impl< T >::value_type;

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

namespace nv
{
    namespace meta
    {
        /**
         * @brief Simulates loop like template instantiation
         *
         * @tparam F - Condition
         * @tparam CF - Call on fail
         * @tparam C - Call on success
         */
        template < typename F, template < typename... > class CF, typename C = listify<> >
        struct do_while
        {
        };

        namespace detail
        {
            template < int N, typename F, template < typename... > class CF, typename C >
            struct dispatch< N, do_while< F, CF, C > >
            {
                template < typename... Ts >
                using f = call< typename conditional< call< F, Ts... >::value >::
                                    template value_type< CF< do_while< F, CF, C > >, C >,
                                Ts... >;
            };

            template < typename F, template < typename... > class CF, typename C >
            struct dispatch< 0, do_while< F, CF, C > >
            {
                template < typename... Ts > using f = call< C, Ts... >;
            };
        } // namespace detail

    } // namespace meta
} // namespace nv

struct v3f
{
    float x;
    float y;
    float z;
};

namespace nv
{
    namespace meta
    {
        struct no_fields_detector
        {
            template < typename T, typename... Ts >
            constexpr static auto detect( int ) -> decltype( T{Ts{}...} );

            template < typename... > constexpr static false_type detect( ... );

            template < typename... Ts >
            using can_instantiate = nv::meta::neg<
                nv::meta::is_same< decltype( detect< Ts... >( 0 ) ), false_type > >;
        };

        struct any_type
        {
            template < typename T > operator T();
        };
    } // namespace meta
} // namespace nv

namespace nv
{
    namespace meta
    {
        template < typename C = listify<> > struct pop_back
        {
        };

        namespace detail
        {
            template < typename C, typename T, typename... Ts > struct pop_back_impl
            {
                template < typename... >
                using f = typename dispatch< sizeof...( Ts ), C >::template f< Ts... >;
            };

            template < typename C > struct dispatch< 0, pop_back< C > >
            {
                template < typename T0 >
                using f = typename dispatch< 0, C >::template f<>;
            };

            template < typename C > struct dispatch< 1, pop_back< C > >
            {
                template < typename T0 >
                using f = typename dispatch< 0, C >::template f<>;
            };

            template < typename C > struct dispatch< 2, pop_back< C > >
            {
                template < typename T0, typename T1 >
                using f = typename dispatch< 1, C >::template f< T0 >;
            };

            template < typename C > struct dispatch< 3, pop_back< C > >
            {
                template < typename T0, typename T1, typename T2 >
                using f = typename dispatch< 2, C >::template f< T0, T1 >;
            };

            template < int N, typename C > struct dispatch< N, pop_back< C > >
            {
                template < typename... Ts >
                using f =
                    typename dispatch< N + 1, reverse< promote< pop_back_impl > > >::
                        template f< Ts..., reverse< C > >::template f< Ts... >;
            };
        } // namespace detail

    } // namespace meta
} // namespace nv

template < typename... Ts > using calc_len = nv::meta::int_type< sizeof...( Ts ) >;

template < typename... Ts >
using can_instantiate =
    typename nv::meta::no_fields_detector::template can_instantiate< Ts... >;

template < typename T >
using get_fields_list = nv::meta::call<
    nv::meta::gen_n_types<
        sizeof( T ), nv::meta::any_type,
        nv::meta::do_while< nv::meta::promote< can_instantiate >, nv::meta::pop_back > >,
    T >;

// using pop_back_test =
//     nv::meta::call< nv::meta::pop_back<>, int, float, char, float, short >;
template < typename T, typename... Ts >
constexpr auto at0( nv::meta::type_list< T, Ts... > && ) -> T;

/**
 * @brief Calculates number of fields of given type
 *
 * Works only for types that are allowed to be constructed using {}
 *
 * @tparam T - type
 * @return constexpr int - number of fields or -1
 */
template < typename T > constexpr int calculate_number_of_fields()
{
    using fields_list = get_fields_list< T >;
    using head        = decltype( at0( fields_list{} ) );
    constexpr int len =
        nv::meta::conditional< nv::meta::is_same< head, nv::meta::false_type >::value >::
            template value_type<
                nv::meta::call< nv::meta::unpack< nv::meta::promote< calc_len > >,
                                fields_list >,
                nv::meta::int_type< 0 > >::value;
    return len - 1; // first element is a type we calculate the number of fields for
}

namespace detail
{
    /**
     * @brief Used for getting back the information about the type
     * We are going to use a very simple, one might say crud way of getting
     * back the information about the type.
     */
    template < int I > struct any_type_with_backward_info
    {
        int* m_value = nullptr;

        template < typename T > constexpr operator T() noexcept
        {
            m_value[I] = I;
            return T{};
        }
    };
} // namespace detail

namespace nv
{
    template < typename T, int N > struct static_array
    {
      public:
        constexpr static_array() noexcept {};

        constexpr T& operator[]( int i ) noexcept { return m_data[i]; }

        constexpr const T& operator[]( int i ) const noexcept { return m_data[i]; }

        constexpr T* ptr() noexcept { return &m_data[0]; }

        constexpr T* ptr() const noexcept { return &m_data[0]; }

        constexpr int size() noexcept { return N; }

      private:
        T m_data[N] = {0};
    };
} // namespace nv

namespace nv
{
    namespace meta
    {
        /**
         * @brief Register all the types that will be used via the type system
         *
         * Types are stored within the list of the lists with corresponding id
         */
        using fundamental_types = call< zip_with_index<>, int64_t, uint64_t, int32_t,
                                        uint32_t, int16_t, uint16_t, int8_t, uint8_t >;
    } // namespace meta
} // namespace nv

template < typename T > constexpr auto dismantle_types() noexcept
{
    auto types_holder = nv::static_array< int, 3 >{};
    ( void )T{detail::any_type_with_backward_info< 0 >{types_holder.ptr()},
              detail::any_type_with_backward_info< 1 >{types_holder.ptr()},
              detail::any_type_with_backward_info< 2 >{types_holder.ptr()}};

    return types_holder[0] + types_holder[1] + types_holder[2];
}

inline void test()
{
    logger::log( "Number of fields: ", calculate_number_of_fields< v3f >() );
}

static_assert( dismantle_types< v3f >() == 3, "" );

static_assert( can_instantiate< v3f, nv::meta::any_type, nv::meta::any_type,
                                nv::meta::any_type >::value,
               "" );