#pragma once 

#include "meta.hpp"

/**
 * Add ablity to describe types and generate code using that description
 * i.e. generate vertex buffers/arrays using vertex type
 */

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


