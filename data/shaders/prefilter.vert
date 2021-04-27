#version 450

layout (location = 0) in vec2 in_position;

layout (std140, binding = 0) uniform CameraData {
    mat4 proj;
    mat4 view;
};

layout (location = 0) out vec2 var_position;

void main() {
    var_position = in_position;
    gl_Position = vec4(in_position, 0.0, 1.0);

#ifdef VULKAN
    gl_Position.y = -gl_Position.y;
#endif
}
