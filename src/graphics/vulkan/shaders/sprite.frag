#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec2 i_texcoord;
layout (location = 1) in vec4 i_color;
layout (location = 2) flat in uint i_instance_id;

struct texture_data {
    int valid;
    vec2 size;
};

struct sprite_data {
    mat2x3 transform;
    int visible;
    int texture_id;
};

layout (set = 0, binding = 1) buffer textures {
    texture_data data[];
} u_textures;

layout (set = 0, binding = 2) buffer sprites {
    sprite_data data[];
} u_sprites;

layout (set = 0, binding = 3) uniform sampler2D u_samplers[];

layout (location = 0) out vec4 o_color;

void main() {
    o_color = texture(u_samplers[u_sprites.data[i_instance_id].texture_id], i_texcoord);
}
