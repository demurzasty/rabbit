#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec2 i_texcoord;
layout (location = 1) in vec4 i_color;

layout (set = 0, binding = 3) uniform sampler2D u_samplers[];

layout (push_constant) uniform push_constant {
    int texture_index;
} u_push_constant;

layout (location = 0) out vec4 o_color;

void main() {
    if (u_push_constant.texture_index > -1) {
        o_color = i_color * texture(u_samplers[u_push_constant.texture_index], i_texcoord);
    } else {
        o_color = i_color;
    }
}
