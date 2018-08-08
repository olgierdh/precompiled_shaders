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

template < typename T > struct f_on_type_list;

template < typename... Ts > struct f_on_type_list< type_list< Ts... > >
{
    template < template < template < typename... > class, typename... >
               class F0,
               template < typename... > class F >
    using value_type = F0< F, Ts... >;
};

template < template < typename... > class F > struct reducer
{
    template < int size, typename R, typename T > struct dispatch;

    template < int size, typename R, typename H, typename... Ts >
    struct dispatch< size, R, type_list< H, Ts... > >
    {
        using value_type =
            reducer< F >::dispatch< size - 1,
                                    typename F< R, H >::value_type,
                                    type_list< Ts... > >;
    };

    template < typename R > struct dispatch< 0, R, type_list<> >
    {
        using value_type = R;
    };

    template < typename H, typename H2, typename... Ts >
    using value_type = typename reducer< F >::dispatch< sizeof...( Ts ),
                                               typename F< H, H2 >::value_type,
                                               type_list< Ts... > >::value_type;
};

template < template < typename... > class F, typename H, typename... Ts >
using reduce = typename reducer< F >::template value_type< Ts... >;

