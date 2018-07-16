#version 450 core

layout(location = 0) in vec3 vs_pos_ms;

void main()
{
    gl_Position.xyz = vs_pos_ms;
    gl_Position.w = 1.0;
}
