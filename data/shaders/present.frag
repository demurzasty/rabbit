#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 v_texcoord;

layout (binding = 0) uniform sampler2D u_color_map;

layout (location = 0) out vec4 out_color;

void main() {
    out_color = vec4(texture(u_color_map, v_texcoord).rgb, 1.0);
}
