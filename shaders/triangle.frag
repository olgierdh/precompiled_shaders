#version 460 core

layout(location = 0) out vec4 out_color;
layout(location = 1) in vec4 in_color;

void main()
{
    out_color = in_color;
}
