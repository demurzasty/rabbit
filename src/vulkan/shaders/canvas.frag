#version 460 core

layout (location = 0) out vec4 o_color;

layout (location = 0) in vec2 i_texcoord;
layout (location = 1) in vec4 i_color;

void main() {
    o_color = i_color;
}
