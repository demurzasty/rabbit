#version 450

layout (location = 0) in vec2 in_position;

layout (std140, binding = 0) uniform Matrices {
    mat4 world;
    mat4 view;
    mat4 proj;
};

layout (location = 0) out vec2 var_position;
layout (location = 1) out vec2 var_texcoord;

void main() {
    var_position = in_position;
    var_texcoord = in_position;
    gl_Position = vec4(in_position, 0.0, 1.0);

#ifdef VULKAN
    gl_Position.y = -gl_Position.y;
#endif
}
