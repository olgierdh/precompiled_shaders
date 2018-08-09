#pragma once

template < bool > struct conditional
{
    template < typename T, typename U > using value_type = T;
};

template <> struct conditional< false >
{
    template < typename T, typename U > using value_type = U;
};

struct true_type
{
    static constexpr bool value = true;
};

struct false_type
{
    static constexpr bool value = false;
};

struct empty_type {};

template < typename T, typename R > struct is_same
{
    using value_type = false_type;
};

template < typename T > struct is_same< T, T >
{
    using value_type = true_type;
};

template < typename... T > struct type_list
{
};

namespace detail
{
    template < typename H, typename... Ts >
    constexpr H get_head_impl( type_list< H, Ts... >&& );

    template < typename H, typename H1, typename... Ts >
    constexpr H1 get_next_head_impl( type_list< H, H1, Ts... >&& );

    template < typename H, typename... Ts >
    constexpr type_list< Ts... > get_tail_impl( type_list< H, Ts... >&& );

    template< typename... Ts >
    constexpr int get_len_impl( type_list< Ts... >&& )
    {
        return sizeof...( Ts );
    }
} // namespace detail

template < typename T >
using get_head = decltype( detail::get_head_impl( T{} ) );

template < typename T >
using get_next_head = decltype( detail::get_next_head_impl( T{} ) );

template < typename T >
using get_tail = decltype( detail::get_tail_impl( T{} ) );

template< typename T >
constexpr auto get_len = detail::get_len_impl( T{} );

template < template < typename... > class F > struct foreacher
{
    template < typename T > using value_type = typename F< T >::value_type;
};

template < template < typename... > class F, typename... Ts >
using foreach =
    type_list< typename foreacher< F >::template value_type< Ts >... >;

// Prototype for calling f with a functor on a type_list
template < typename T > struct f_on_type_list;

namespace detail
{
    template < int size > struct reduce_dispatch
    {
        template < template < typename... > class F, typename R, typename T >
        using value_type =
            typename reduce_dispatch< size - 1 >::template value_type<
                F,
                typename F< R, get_head< T > >::value_type,
                get_tail< T > >;
    };

    template <> struct reduce_dispatch< 0 >
    {
        template < template < typename... > class F, typename R, typename T >
        using value_type = R;
    };

    template < int size > struct until_dispatch
    {
        template < template < typename... > class F, typename T >
        using value_type =
            typename conditional< F< get_head< T > >::value_type::value >::
                template value_type<
                    typename F< get_head< T > >::value_type,
                    typename until_dispatch<
                        size - 1 >::template value_type< F, get_tail< T > > >;
    };

    template <> struct until_dispatch< 1 >
    {
        template < template < typename... > class F, typename T >
        using value_type = typename F< get_head< T > >::value_type;
    };
} // namespace detail

template < template < typename... > class F > struct finder
{
    template < typename T >
    using value_type = typename detail::until_dispatch<
        get_len< T > >::template value_type< F, T >;
};

template < template < typename... > class F, typename T >
using find_if = typename finder< F >::template value_type< T >;

/**
 * Reducing lists using functor
 *
 * Implementation uses the intermediate dispatcher for calculating the
 * number of elements left within the list.
 */
template < template < typename... > class F > struct reducer
{
    template < typename T >
    using value_type = typename detail::reduce_dispatch< get_len< T > >::
        template value_type<
            F, empty_type, T >;
};

/**
 * For simplified calling
 */
template < template < typename... > class F, typename T >
using reduce = typename reducer< F >::template value_type< T >;


// integer sequences
template < int... Is > struct integer_sequence
{
};

namespace detail
{
    template < int U, int... Ts >
    using push_back = integer_sequence< Ts..., U >;

    template < typename T > struct push_back_on_integer_sequence_impl;

    template < int... Ts >
    struct push_back_on_integer_sequence_impl< integer_sequence< Ts... > >
    {
        template < int U > using value_type = push_back< U, Ts... >;
    };
} // namespace detail

template < int U, typename T >
using push_back_on_integer_sequence =
    typename detail::push_back_on_integer_sequence_impl<
        T >::template value_type< U >;

namespace detail
{
    template < int N > struct construct_integer_sequence_impl
    {
        template < int C, typename T >
        using value_type = typename construct_integer_sequence_impl< N - 1 >::
            template value_type< C + 1, push_back_on_integer_sequence< C, T > >;
    };

    template <> struct construct_integer_sequence_impl< 0 >
    {
        template < int C, typename T > using value_type = T;
    };
} // namespace detail

template < int N >
using construct_integer_sequence =
    typename detail::construct_integer_sequence_impl<
        N >::template value_type< 0, integer_sequence<> >;
