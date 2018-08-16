// Copyright (C) 2017-2018 ChaosForge Ltd
// http://chaosforge.org/
//
// This file is part of Nova libraries. 
// For conditions of distribution and use, see copying.txt file in root folder.

#pragma once

#include <array>

#include <GL/glew.h>

#include "logger.hpp"
#include "meta.hpp"

namespace detail
{
    template < typename T >
    using get_value_type = typename T::value_type::value_type;

    template < typename T >
    using get_field_list = typename T::type_desc::field_list;

    template < typename T > using get_field_desc = typename T::value_type;

    template < typename... T >
    using calc_sizeof = nv::meta::int_type< ( sizeof( T ) + ... ) >;

    template < typename... T >
    using calc_len = nv::meta::int_type< sizeof...( T ) >;

    template < typename LHS, typename RHS > struct type_equality
    {
        using value_type = typename nv::meta::conditional<
            nv::meta::is_same< LHS, RHS >::value >::
            template value_type< nv::meta::false_type, LHS >;
    };

    template < typename RHS > struct type_equality< nv::meta::false_type, RHS >
    {
        using value_type = nv::meta::false_type;
    };

    template < typename RHS > struct type_equality< nv::meta::null_type, RHS >
    {
        using value_type = RHS;
    };

    template < typename T, typename U >
    int64_t offsetOf( U T::*const member ) noexcept
    {
        const auto t = T{};
        return reinterpret_cast< int64_t >( &( t.*member ) ) -
               reinterpret_cast< int64_t >( &t );
    }
} // namespace detail


// create per channel descriptors
enum class gl_type
{
    float_type,
    byte_type,
    unsigned_byte_type,
    short_type,
    unknown
};

template < typename T > constexpr gl_type get_gl_type( T&& )
{
    using type = T;

    if constexpr ( nv::meta::is_same< type, float >::value )
    {
        return gl_type::float_type;
    }
    else if constexpr ( nv::meta::is_same< type, char >::value )
    {
        return gl_type::byte_type;
    }
    else if constexpr ( nv::meta::is_same< type, unsigned char >::value )
    {
        return gl_type::unsigned_byte_type;
    }
    else if constexpr ( nv::meta::is_same< type, short >::value )
    {
        return gl_type::short_type;
    }

    return gl_type::unknown;
}

GLenum gl_type_to_gl_enum( gl_type type )
{
    switch ( type )
    {
        case gl_type::float_type:
            return GL_FLOAT;
        case gl_type::byte_type:
            return GL_BYTE;
        case gl_type::unsigned_byte_type:
            return GL_UNSIGNED_BYTE;
        case gl_type::short_type:
            return GL_SHORT;
        default:
            return GL_FIXED;
    };

    return GL_FIXED;
}

struct channel
{
    gl_type m_type;
    int64_t m_size;
    int64_t m_offset;
    int64_t m_len;
};

template < typename T > struct gl_vao_channel_desc_generator
{
    using flatten_fields_list =
        nv::meta::call< nv::meta::unpack< nv::meta::foreach<
                            nv::meta::promote< detail::get_field_list > > >,
                        typename T::field_list >;

    using flatten_fields_desc_list =
        nv::meta::call< nv::meta::unpack< nv::meta::foreach<
                            nv::meta::promote< detail::get_field_desc > > >,
                        typename T::field_list >;
    using value_type = T;

    static constexpr int get_no_channels() noexcept
    {
        return nv::meta::call<
            nv::meta::unpack< nv::meta::promote< detail::calc_len > >,
            flatten_fields_list >::value;
    }

    static decltype( auto ) generate_channels() noexcept
    {
        // nv::meta::test< top_fields_desc_list >();
        return generate_channels_impl( flatten_fields_list{},
                                       flatten_fields_desc_list{} );
    }

  private:
    template < typename A, typename B >
    static channel generate_channel( A&&, B&& ) noexcept
    {
        const auto offset = detail::offsetOf( B::member_ptr );
        constexpr auto size =
            nv::meta::call< nv::meta::unpack< nv::meta::foreach<
                                nv::meta::promote< detail::get_value_type >,
                                nv::meta::promote< detail::calc_sizeof > > >,
                            A >::value;

        // reduce, yeah I could generate second list and check if equal, but
        // this is more fun.
        using the_type =
            nv::meta::call< nv::meta::unpack< nv::meta::foreach<
                                nv::meta::promote< detail::get_value_type >,
                                nv::meta::reduce< detail::type_equality > > >,
                            A >;

        constexpr auto len = nv::meta::call<
            nv::meta::unpack< nv::meta::promote< detail::calc_len > >,
            A >::value;

        static_assert(
            nv::meta::is_same< nv::meta::false_type, the_type >::value != true,
            "Channel's types are not the same!" );

        return channel{get_gl_type( the_type{} ), size, offset, len};
    }

    template < typename... A, typename... B >
    static decltype( auto )
    generate_channels_impl( nv::meta::type_list< A... >&&,
                            nv::meta::type_list< B... >&& ) noexcept
    {
        std::array< channel, sizeof...( A ) > channels = {
            generate_channel( A{}, B{} )...};
        return channels;
    }
};

