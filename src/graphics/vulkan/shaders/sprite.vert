#version 460 core

layout (location = 0) out vec2 o_texcoord;
layout (location = 1) out vec4 o_color;

void main() {
    o_texcoord = vec2(0.0);
    o_color = vec4(1.0);
    gl_Position = vec4(0, 0, 0, 1);
}
