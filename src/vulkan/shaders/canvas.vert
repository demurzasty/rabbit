#version 460 core

layout (location = 0) in vec2 i_position;
layout (location = 1) in vec2 i_texcoord;
layout (location = 2) in vec4 i_color;

layout (location = 0) out vec2 o_texcoord;
layout (location = 1) out vec4 o_color;

void main() {
    o_texcoord = i_texcoord;
    o_color = i_color;
    gl_Position = vec4(i_position, 0, 1);
}
