#pragma once

#include <array>

#include "type_system.hpp"
#include "logger.hpp"

/* our building blocks */
struct vec3f
{
    float x;
    float y;
    float z;
};

struct vec4f
{
    float x;
    float y;
    float z;
    float w;
};

struct vec4c
{
    char r;
    char g;
    char b;
    char a;
};

struct vertex
{
    vec4f m_position;
    vec4c m_color0;
    vec3f m_color1;
};

/* meta description of types not very beautiful but handy later on */
using vec3f_desc =
    decltype( make_struct_desc( vec3f{},
                                "vec3f"_tstr,
                                make_field_desc( &vec3f::x, "x"_tstr ),
                                make_field_desc( &vec3f::y, "y"_tstr ),
                                make_field_desc( &vec3f::z, "z"_tstr ) ) );

using vec4f_desc =
    decltype( make_struct_desc( vec4f{},
                                "vec4f"_tstr,
                                make_field_desc( &vec4f::x, "x"_tstr ),
                                make_field_desc( &vec4f::y, "y"_tstr ),
                                make_field_desc( &vec4f::z, "z"_tstr ),
                                make_field_desc( &vec4f::w, "w"_tstr ) ) );
using vec4c_desc =
    decltype( make_struct_desc( vec4c{},
                                "vec4c"_tstr,
                                make_field_desc( &vec4c::r, "r"_tstr ),
                                make_field_desc( &vec4c::g, "g"_tstr ),
                                make_field_desc( &vec4c::b, "b"_tstr ),
                                make_field_desc( &vec4c::a, "a"_tstr ) ) );


using vertex_desc = decltype( make_struct_desc(
    vertex{},
    "vertex"_tstr,
    make_field_desc( &vertex::m_position, "m_position"_tstr, vec4f_desc{} ),
    make_field_desc( &vertex::m_color0, "m_color0"_tstr, vec4c_desc{} ),
    make_field_desc( &vertex::m_color1, "m_color1"_tstr, vec3f_desc{} ) ) );

/* list of types registered for renderer system */
using renderer_reflection =
    nv::meta::type_list< vec3f_desc, vec4f_desc, vertex_desc >;

/** bunch of helpers to generate the vertex array object from the vertex
 * description */


// iterate over the fields and replace the types with apropriate generator
// ran afterwards will give us the configuration for the vertex arrays


// create per channel descriptors
template < typename T >
using get_value_type = typename T::value_type::value_type;

template < typename T >
using get_field_list = typename T::type_desc::field_list;

enum class gl_type
{
    float_type,
    int_type,
    char_type,
    unknown
};

template < typename T > constexpr gl_type get_gl_type( T&& )
{
    using type = T;

    if constexpr ( nv::meta::is_same< type, float >::value )
    {
        return gl_type::float_type;
    }
    else if constexpr ( nv::meta::is_same< type, int >::value )
    {
        return gl_type::int_type;
    }
    else if constexpr ( nv::meta::is_same< type, char >::value )
    {
        return gl_type::char_type;
    }

    return gl_type::unknown;
}

struct channel
{
    gl_type m_type;
    int m_size;
};

template < typename... T >
using calc_sizeof = nv::meta::int_type< ( sizeof( T ) + ... ) >;

template < typename... T >
using calc_len = nv::meta::int_type< sizeof...( T ) >;

template < typename LHS, typename RHS > struct type_equality
{
    using value_type =
        typename nv::meta::conditional< nv::meta::is_same< LHS, RHS >::value >::
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

template < typename T > struct channels
{
    using flatten_fields_list =
        nv::meta::call< nv::meta::unpack< nv::meta::foreach<
                            nv::meta::promote< get_field_list > > >,
                        typename T::field_list >;

    channels()
    {
        logger::log( "Test: ", get_no_channels() );
    }

    static constexpr int get_no_channels()
    {
        return nv::meta::call<
            nv::meta::unpack< nv::meta::promote< calc_len > >,
            flatten_fields_list >::value;
    }

    static auto generate_channels()
    {
        return generate_channels_impl( flatten_fields_list{} );
    }

  private:
    template < typename A > static channel generate_channel( A&& )
    {
        constexpr auto size =
            nv::meta::call< nv::meta::unpack< nv::meta::foreach<
                                nv::meta::promote< get_value_type >,
                                nv::meta::promote< calc_sizeof > > >,
                            A >::value;

        // reduce, yeah I could generate second list and check if equal, but
        // this is more fun.
        using the_type =
            nv::meta::call< nv::meta::unpack< nv::meta::foreach<
                                nv::meta::promote< get_value_type >,
                                nv::meta::reduce< type_equality > > >,
                            A >;
        static_assert(
            nv::meta::is_same< nv::meta::false_type, the_type >::value != true,
            "Channel's types are not equal!" );

        return channel{get_gl_type( the_type{} ), size};
    }

    template < typename... A >
    static auto generate_channels_impl( nv::meta::type_list< A... >&& )
    {
        std::array< channel, sizeof...( A ) > channels = {
            generate_channel( A{} )...};
        return channels;
    }
};

