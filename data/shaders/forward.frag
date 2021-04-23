#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 v_texcoord;

layout (binding = 2) uniform MaterialData {
    vec3 u_base_color;
    float u_roughness;
    float u_metallic;
};

layout(binding = 3) uniform sampler2D u_albedo_map;

layout (location = 0) out vec4 out_color;

void main() {
    out_color = vec4(u_base_color * texture(u_albedo_map, v_texcoord).rgb, 1.0);
}
