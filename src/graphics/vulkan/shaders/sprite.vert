#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec2 i_position;
layout (location = 1) in vec2 i_texcoord;
layout (location = 2) in vec4 i_color;

layout (location = 0) out vec2 o_texcoord;
layout (location = 1) out vec4 o_color;
layout (location = 2) flat out uint o_instance_id;

struct texture_data {
    vec2 size;
    int valid;
};

struct sprite_data {
    mat3x2 transform;
    int visible;
    int texture_id;
};

layout (std430, set = 0, binding = 1) buffer textures {
    texture_data data[];
} u_textures;

layout (std430, set = 0, binding = 2) buffer sprites {
    sprite_data data[];
} u_sprites;

layout (set = 0, binding = 3) uniform sampler2D u_samplers[];

void main() {
    vec2 position = i_position * u_textures.data[u_sprites.data[gl_InstanceIndex].texture_id].size;
    vec2 position2 = u_sprites.data[gl_InstanceIndex].transform * vec3(position, 1.0);
    //vec3 position2 = mat2x3(1.0) * position;
    position = position2.xy;// / position2.z;

    o_texcoord = i_texcoord;
    o_color = o_color;
    gl_Position = vec4(position / vec2(1280.0, 720.0) * 2.0 - 1.0, 0, 1);
}
