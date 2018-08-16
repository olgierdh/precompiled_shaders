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
                                make_field_desc< &vec3f::x >( "x"_tstr ),
                                make_field_desc< &vec3f::y >( "y"_tstr ),
                                make_field_desc< &vec3f::z >( "z"_tstr ) ) );

using vec4f_desc =
    decltype( make_struct_desc( vec4f{},
                                "vec4f"_tstr,
                                make_field_desc< &vec4f::x >( "x"_tstr ),
                                make_field_desc< &vec4f::y >( "y"_tstr ),
                                make_field_desc< &vec4f::z >( "z"_tstr ),
                                make_field_desc< &vec4f::w >( "w"_tstr ) ) );
using vec4c_desc =
    decltype( make_struct_desc( vec4c{},
                                "vec4c"_tstr,
                                make_field_desc< &vec4c::r >( "r"_tstr ),
                                make_field_desc< &vec4c::g >( "g"_tstr ),
                                make_field_desc< &vec4c::b >( "b"_tstr ),
                                make_field_desc< &vec4c::a >( "a"_tstr ) ) );


using vertex_desc = decltype( make_struct_desc(
    vertex{},
    "vertex"_tstr,
    make_field_desc< &vertex::m_position >( "m_position"_tstr, vec4f_desc{} ),
    make_field_desc< &vertex::m_color0 >( "m_color0"_tstr, vec4c_desc{} ),
    make_field_desc< &vertex::m_color1 >( "m_color1"_tstr, vec3f_desc{} ) ) );

/* list of types registered for renderer system */
using renderer_reflection =
    nv::meta::type_list< vec3f_desc, vec4f_desc, vertex_desc >;


