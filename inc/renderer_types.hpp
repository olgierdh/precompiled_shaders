#pragma once

#include "type_system.hpp"

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

/* meta description of types */
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

using vertex_desc = decltype(
    make_struct_desc( vertex{},
                      "vertex"_tstr,
                      make_field_desc( &vertex::m_position, "m_position"_tstr ),
                      make_field_desc( &vertex::m_color, "m_color"_tstr ) ) );

/* list of types registered for renderer system */
using renderer_reflection = type_list< vec3f_desc, vec4f_desc, vertex_desc >;

