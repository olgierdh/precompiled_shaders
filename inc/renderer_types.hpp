// Copyright (C) 2017-2018 ChaosForge Ltd
// http://chaosforge.org/
//
// This file is part of Nova libraries. 
// For conditions of distribution and use, see copying.txt file in root folder.

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
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct vertex
{
    vec4f m_position;
    vec4f m_color0;
};

string_literal< _T( "vec3f" ) > vec3f_sl;
string_literal< _T( "vec4f" ) > vec4f_sl;
string_literal< _T( "vec4c" ) > vec4c_sl;
string_literal< _T( "x" ) >     x_sl;
string_literal< _T( "y" ) >     y_sl;
string_literal< _T( "z" ) >     z_sl;
string_literal< _T( "w" ) >     w_sl;
string_literal< _T( "r" ) >     r_sl;
string_literal< _T( "g" ) >     g_sl;
string_literal< _T( "b" ) >     b_sl;
string_literal< _T( "a" ) >     a_sl;

/* meta description of types not very beautiful but handy later on */
using vec3f_desc =
    decltype( make_struct_desc( vec3f{},
                                vec3f_sl,
                                make_field_desc< &vec3f::x >( x_sl ),
                                make_field_desc< &vec3f::y >( y_sl ),
                                make_field_desc< &vec3f::z >( z_sl ) ) );

using vec4f_desc =
    decltype( make_struct_desc( vec4f{},
                                vec4f_sl,
                                make_field_desc< &vec4f::x >( x_sl ),
                                make_field_desc< &vec4f::y >( y_sl ),
                                make_field_desc< &vec4f::z >( z_sl ),
                                make_field_desc< &vec4f::w >( w_sl ) ) );
using vec4c_desc =
    decltype( make_struct_desc( vec4c{},
                                vec4c_sl,
                                make_field_desc< &vec4c::r >( r_sl ),
                                make_field_desc< &vec4c::g >( g_sl ),
                                make_field_desc< &vec4c::b >( b_sl ),
                                make_field_desc< &vec4c::a >( a_sl ) ) );


string_literal< _T("vertex") >     vertex_sl;
string_literal< _T("m_position") > m_position_sl;
string_literal< _T("m_color0") >   m_color0_sl;


using vertex_desc = decltype( make_struct_desc(
    vertex{},
    vertex_sl,
    make_field_desc< &vertex::m_position >( m_position_sl, vec4f_desc{} ),
    make_field_desc< &vertex::m_color0 >( m_color0_sl, vec4f_desc{} ) ) );

/* list of types registered for renderer system */
using renderer_reflection =
    nv::meta::type_list< vec3f_desc, vec4f_desc, vertex_desc >;

