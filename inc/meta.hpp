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

struct empty_type
{
};

namespace detail
{
    template < typename T, typename R > struct is_same
    {
        using value_type = false_type;
    };

    template < typename T > struct is_same< T, T >
    {
        using value_type = true_type;
    };
} // namespace detail

template< typename T, typename R >
using is_same = typename detail::is_same< T, R >::value_type; 

template < typename... T > struct type_list
{
};

template< typename T, T v >
struct valuer
{
    using value_type = T;
    constexpr static auto value = v;
};

template < int... Is > struct integer_sequence
{
};

namespace detail
{
    template < typename T > constexpr false_type is_integer_sequence( T&& );

    template < int... Is >
    constexpr true_type is_integer_sequence( integer_sequence< Is... >&& );
} // namespace detail

template < typename T >
using is_integer_sequence = decltype( detail::is_integer_sequence( T{} ) );

namespace detail
{
    template < int i, int... Is >
    constexpr auto
    push_back_on_integer_sequence_impl( int, integer_sequence< Is... > && )
        -> integer_sequence< Is..., i >;
} // namespace detail

template < int U, typename T >
using push_back_on_integer_sequence =
    decltype( detail::push_back_on_integer_sequence_impl< U >( U, T{} ) );

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


template < typename T, int index > struct type_with_index
{
    using value_type                 = T;
    constexpr static int value_index = index;
};

namespace detail
{
    template < bool > struct enable_if
    {
        template < typename T = void > using value_type = T;
    };

    template <> struct enable_if< false >
    {
    };
} // namespace detail

template < bool c, typename T = void >
using enable_if = typename detail::enable_if< c >::template value_type< T >;

namespace detail
{
    template < typename H, typename... Ts >
    constexpr H get_head_impl( type_list< H, Ts... >&& );

    template < int H, int... Ts >
    constexpr int get_head_impl( integer_sequence< H, Ts... >&& )
    {
        return H;
    }

    template < typename H, typename... Ts >
    constexpr type_list< Ts... > get_tail_impl( type_list< H, Ts... >&& );

    template < int H, int... Ts >
    constexpr integer_sequence< Ts... >
    get_tail_impl( integer_sequence< H, Ts... >&& );

    template < typename... Ts >
    constexpr int get_len_impl( type_list< Ts... >&& )
    {
        return sizeof...( Ts );
    }

    template < typename T, typename... Ts >
    constexpr auto push_back_on_list_impl( T&&, type_list< Ts... > && )
        -> type_list< Ts..., T >;
} // namespace detail

/** needed because of the compilers bug for matching the aliased templates */
template < typename T >
using get_head = decltype( detail::get_head_impl( T{} ) );

template < typename T >
using get_tail = decltype( detail::get_tail_impl( T{} ) );

template < typename T >
constexpr auto get_head_int = detail::get_head_impl( T{} );

template < typename T > constexpr auto get_len = detail::get_len_impl( T{} );

template < typename T, typename L >
using push_back_on_list =
    decltype( detail::push_back_on_list_impl( T{}, L{} ) );


namespace detail
{
    template < int N > struct construct_type_list_impl
    {
        template < typename H, typename T >
        using value_type = typename construct_type_list_impl< N - 1 >::
            template value_type< H, push_back_on_list< H, T > >;
    };

    template <> struct construct_type_list_impl< 0 >
    {
        template < typename H, typename T > using value_type = T;
    };
} // namespace detail

template < int N, typename H >
using construct_type_list = typename detail::construct_type_list_impl<
    N >::template value_type< H, type_list<> >;

constexpr int cmin( int lhs, int rhs )
{
    if ( lhs < rhs )
    {
        return lhs;
    }
    return rhs;
}

namespace detail
{
    template < int size > struct foreach_dispatcher
    {
        template < template < typename... > class F, typename R, typename T >
        using value_type =
            typename foreach_dispatcher< size - 1 >::template value_type<
                F,
                push_back_on_list< typename F< get_head< T > >::value_type, R >,
                get_tail< T > >;
    };

    template <> struct foreach_dispatcher< 0 >
    {
        template < template < typename... > class F, typename R, typename T >
        using value_type = R;
    };
} // namespace detail

template < template < typename... > class F, typename T >
using foreach = typename detail::foreach_dispatcher<
    get_len< T > >::template value_type< F, type_list<>, T >;

namespace detail
{
    template < int size > struct transform_dispatch
    {
        template < template < typename... > class F,
                   typename R,
                   typename T0,
                   typename T1 >
        using value_type =
            typename transform_dispatch< size - 1 >::template value_type<
                F,
                push_back_on_list<
                    typename F< get_head< T0 >, get_head< T1 > >::value_type,
                    R >,
                get_tail< T0 >,
                get_tail< T1 > >;
    };

    template <> struct transform_dispatch< 0 >
    {
        template < template < typename... > class F,
                   typename R,
                   typename T0,
                   typename T1 >
        using value_type = R;
    };
} // namespace detail

template < template < typename... > class F, typename T0, typename T1 >
using transform = typename detail::transform_dispatch< cmin(
    get_len< T0 >,
    get_len< T1 > ) >::template value_type< F, type_list<>, T0, T1 >;


namespace detail
{
    template < int size > struct zip_with_index_dispatch
    {
        template < typename R, typename T0, typename T1 >
        using value_type =
            typename zip_with_index_dispatch< size - 1 >::template value_type<
                push_back_on_list<
                    type_with_index< get_head< T0 >, get_head_int< T1 > >,
                    R >,
                get_tail< T0 >,
                get_tail< T1 > >;
    };

    template <> struct zip_with_index_dispatch< 0 >
    {
        template < typename R, typename T0, typename T1 > using value_type = R;
    };
} // namespace detail

template < typename T >
using zip_with_integer_sequence =
    typename detail::zip_with_index_dispatch< get_len< T > >::
        template value_type< type_list<>,
                             T,
                             construct_integer_sequence< get_len< T > > >;

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
    using value_type = typename detail::reduce_dispatch<
        get_len< T > >::template value_type< F, empty_type, T >;
};

/**
 * For simplified calling
 */
template < template < typename... > class F, typename T >
using reduce = typename reducer< F >::template value_type< T >;


// bunch of helper structures

/**
 * Generates the comparator for any given type.
 * Can be used against the find_if
 */
template < typename T > struct type_comparator
{
    template < typename R > struct comparator
    {
        using value_type = is_same< T, R >;
    };
};

/**
 * Syntactic sugar for finding a given type within the given type list
 */
template < typename T, typename H >
using find_type_in = find_if< type_comparator< T >::template comparator, H >;

