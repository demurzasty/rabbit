
#version 450

layout (location = 0) in vec3 var_texcoord;

layout (binding = 1) uniform samplerCube skybox;

layout (location = 0) out vec4 out_color;

void main() {
	out_color = texture(skybox, var_texcoord);
}
