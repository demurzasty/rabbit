#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in vec3 in_normal;

layout (binding = 0) uniform CameraData {
    mat4 proj;
    mat4 view;
};

layout (binding = 1) uniform LocalData {
    mat4 world;
};

layout (location = 0) out vec2 v_texcoord;

void main() {
    v_texcoord = in_texcoord;
    gl_Position = proj * view * world * vec4(in_position, 1.0);

#ifdef VULKAN
    gl_Position.y = -gl_Position.y;
#endif
}
