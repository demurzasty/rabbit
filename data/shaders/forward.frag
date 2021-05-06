#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 v_texcoord;
layout (location = 1) in vec3 v_normal;

layout (std140, binding = 2) uniform MaterialData {
    vec3 u_base_color;
    float u_roughness;
    float u_metallic;
};

layout(binding = 3) uniform sampler2D u_albedo_map;
layout(binding = 4) uniform samplerCube u_radiance_map;
layout(binding = 5) uniform samplerCube u_irradiance_map;
layout(binding = 6) uniform samplerCube u_prefilter_map;

layout (location = 0) out vec4 out_color;

void main() {
    out_color = vec4(u_base_color * texture(u_albedo_map, v_texcoord).rgb, 1.0);
    out_color = vec4(texture(u_irradiance_map, v_normal).rgb, 1.0);
    out_color = vec4(texture(u_prefilter_map, v_normal).rgb, 1.0);
}
