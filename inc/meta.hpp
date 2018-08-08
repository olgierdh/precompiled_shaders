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

template < template < typename... > class F > struct foreacher
{
    template < typename T > using value_type = typename F< T >::value_type;
};

template < template < typename... > class F, typename... Ts >
using foreach =
    type_list< typename foreacher< F >::template value_type< Ts >... >;

// Prototype for calling f with a functor on a type_list
template < typename T > struct f_on_type_list;

/**
 * Call F0 with a functor F on a type_list
 */
template < typename... Ts > struct f_on_type_list< type_list< Ts... > >
{
    template < template < template < typename... > class, typename... >
               class F0,
               template < typename... > class F >
    using value_type = F0< F, Ts... >;
};

/**
 * For direct usage - simplifies the calling
 */
template < typename T,
           template < template < typename... > class, typename... > class F0,
           template < typename... > class F >
using call_f_on_a_type_list =
    typename f_on_type_list< T >::template value_type< F0, F >;


namespace detail
{
    template < int size > struct dispatch
    {
        template < template < typename... > class F,
                   typename R,
                   typename H,
                   typename... Ts >
        using value_type = typename dispatch< size - 1 >::
            template value_type< typename F< R, H >::value_type, Ts... >;
    };

    template <> struct dispatch< 0 >
    {
        template < template < typename... > class F, typename R >
        using value_type = R;
    };
} // namespace detail

/**
 * Reducing lists using functor
 *
 * Implementation uses the intermediate dispatcher for calculating the number of
 * elements left within the list.
 */
template < template < typename... > class F > struct reducer
{
    template < typename H, typename H2, typename... Ts >
    using value_type = typename detail::dispatch< sizeof...( Ts ) >::
        template value_type< F, typename F< H, H2 >::value_type, Ts... >;
};

/**
 * For simplified calling
 */
template < template < typename... > class F, typename H, typename... Ts >
using reduce = typename reducer< F >::template value_type< Ts... >;


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
