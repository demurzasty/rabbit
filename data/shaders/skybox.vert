
#version 450

layout (location = 0) in vec3 in_position;

layout (binding = 0) uniform CameraData {
    mat4 proj;
    mat4 view;
};

layout (location = 0) out vec3 var_texcoord;

void main() {
    var_texcoord = normalize(in_position);
    vec4 pos = proj * view * vec4(in_position, 1.0);
    gl_Position = pos.xyww;

#ifdef VULKAN
    gl_Position.y = -gl_Position.y;
#endif
}