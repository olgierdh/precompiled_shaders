#pragma once

#include "meta.hpp"
#include "string_literal.hpp"

/**
 * Add ablity to describe types and generate code using that description
 * i.e. generate vertex buffers/arrays using vertex type
 */
template < typename T, typename S, typename F > struct struct_desc
{
    using value_type = T;
    using value_name = S;
    using field_list = F;
};

template < typename T, typename C > struct field_type
{
    using value_type   = T;
    using context_type = C;
};

template < typename T, typename C >
constexpr auto detect_field_type( T C::* ) -> field_type< T, C >;

template < auto f, typename S, typename D > struct field_desc
{
    constexpr static auto member_ptr = f;
    using value_type                 = decltype( detect_field_type( f ) );
    using value_name                 = S;
    using type_desc                  = D;
};

template < typename... Ts > using fields = nv::meta::type_list< Ts... >;

template < typename T, typename S, typename D >
constexpr auto make_field_desc( T&& t, S&&, D && ) -> field_desc< T{}, S, D >;

template < typename T, typename S >
constexpr auto
make_field_desc( T&& t, S && ) -> field_desc< T{}, S, nv::meta::null_type >;

template < typename T, typename S, typename... Fs >
constexpr auto make_struct_desc( T&&, S&&, Fs&&... )
    -> struct_desc< T, S, nv::meta::type_list< Fs... > >;

