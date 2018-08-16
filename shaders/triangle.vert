#version 460 core

layout(location = 0) in vec4 vs_pos_ms;
layout(location = 1) in vec4 vs_col_ms;
layout(location = 1) out vec4 out_color;

void main()
{
    out_color = vs_col_ms;
    gl_Position.xyzw = vs_pos_ms;
}
