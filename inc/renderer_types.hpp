#pragma once

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

struct vertex
{
    vec4f m_position;
    vec3f m_color;
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

using vertex_desc = decltype( make_struct_desc(
    vertex{},
    "vertex"_tstr,
    make_field_desc( &vertex::m_position, "m_position"_tstr, vec4f_desc{} ),
    make_field_desc( &vertex::m_color, "m_color"_tstr, vec3f_desc{} ) ) );

/* list of types registered for renderer system */
using renderer_reflection = type_list< vec3f_desc, vec4f_desc, vertex_desc >;

/** bunch of helpers to generate the vertex array object from the vertex
 * description */


// iterate over the fields and replace the types with apropriate generator
// ran afterwards will give us the configuration for the vertex arrays

// create per channel descriptors
template < typename T > struct replace_with_list_fields
{
    using value_type = typename T::type_desc::field_list;
};

template < typename T > struct replace_with_field_type
{
    using value_type = typename T::value_type::value_type;
};

template < typename... Ts > struct replace_with_field_type< type_list< Ts... > >
{
    using value_type = foreach< replace_with_field_type, type_list< Ts... > >;
};

template < typename U > struct generate_field_type
{
    using value_type =
        is_same< construct_type_list< get_len< U >, get_head< U > >, U >;
};

template < int size > struct type_size
{
    constexpr static auto value = size;
};

template < typename LHS, typename RHS > struct reducer_field_sizeof
{
    using value_type = type_size< LHS::value + sizeof( RHS ) >;
};

template < typename RHS > struct reducer_field_sizeof< empty_type, RHS >
{
    using value_type = type_size< sizeof( RHS ) >;
};

template < typename T > struct calculate_field_size
{
    using value_type = reduce< reducer_field_sizeof, T >;
};

enum class gl_type
{
    float_type
};

struct channel
{
    gl_type m_type;
    int m_size;
};

template < typename T > struct channels
{
    using flatten_fields_list =
        foreach< replace_with_list_fields, typename T::field_list >;

    channels()
    {
        logger::log( "Test: ", get_no_channels() );
    }

    static constexpr int get_no_channels()
    {
        return get_len< flatten_fields_list >;
    }

    template < typename A > static channel generate_channel( A&& )
    {
        constexpr auto size = reduce< reducer_field_sizeof, A >::value;
        return channel{gl_type::float_type, size};
    }

    template < typename... A >
    static void generate_channels( type_list< A... >&& )
    {
        channel c[sizeof...( A )] = {generate_channel( A{} )...};

        for ( int i = 0; i < static_cast< int >( sizeof...( A ) ); ++i )
        {
            logger::log( "Size: ", c[i].m_size );
        }
    }
};


using flatten_fields_list =
    foreach< replace_with_list_fields, vertex_desc::field_list >;
using flatten_field_type_list =
    foreach< replace_with_field_type, flatten_fields_list >;
using flatten_field_sizes =
    foreach< calculate_field_size, flatten_field_type_list >;

// for verification purposes only
using flatten_reference_type_list =
    foreach< generate_field_type, flatten_field_type_list >;
using all_types_same = is_same<
    construct_type_list< get_len< flatten_reference_type_list >, true_type >,
    flatten_reference_type_list >;
static_assert( is_same< all_types_same, true_type >::value,
               "All types within sub structures must be the same!" );

