#version 450

layout (location = 0) in vec2 in_position;

layout (location = 0) out vec2 v_texcoord;

void main() {
    v_texcoord = in_position * 0.5 + 0.5;
    v_texcoord.y = 1.0 - v_texcoord.y;

    gl_Position = vec4(in_position, 0.0, 1.0);

#ifdef VULKAN
    gl_Position.y = -gl_Position.y;
#endif
}

