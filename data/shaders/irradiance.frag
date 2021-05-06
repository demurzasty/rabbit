#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PI 3.14159265359

layout (location = 0) in vec2 var_position;

layout (std140, binding = 0) uniform IrradianceData {
    int cube_face;
};

layout (binding = 1) uniform samplerCube radianceMap;

layout (location = 0) out vec4 out_color;

void main() {

#if defined VULKAN || defined HLSL
    vec3 N = normalize(vec3(var_position.x, var_position.y, 1));

    if (cube_face == 2) {
        N = normalize(vec3(var_position.x,  1, -var_position.y));
    } else if (cube_face == 3) {
        N = normalize(vec3(var_position.x, -1,  var_position.y));
    } else if (cube_face == 0) {
        N = normalize(vec3(  1, var_position.y,-var_position.x));
    } else if (cube_face == 1) {
        N = normalize(vec3( -1, var_position.y, var_position.x));
    } else if (cube_face == 5) {
        N = normalize(vec3(-var_position.x, var_position.y, -1));
	}
#else
    vec3 N = normalize(vec3(var_position.x, -var_position.y, 1));

    if (cube_face == 2) {
        N = normalize(vec3(var_position.x,  1, var_position.y));
    } else if (cube_face == 3) {
        N = normalize(vec3(var_position.x, -1,  -var_position.y));
    } else if (cube_face == 0) {
        N = normalize(vec3(  1, -var_position.y,-var_position.x));
    } else if (cube_face == 1) {
        N = normalize(vec3( -1, -var_position.y, var_position.x));
    } else if (cube_face == 5) {
        N = normalize(vec3(-var_position.x, -var_position.y, -1));
	}
#endif

	vec3 up = vec3(0,1,0);
    vec3 right = normalize(cross(up, N));
    up = cross(N, right);

	vec3 sampledColour = vec3(0,0,0);
    float index = 0.0;
    for(float phi = 0.0; phi < PI * 2.0; phi += 0.25)
    {
        for(float theta = 0.0; theta < PI * 0.5; theta += 0.025)
        {
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleVector = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            sampledColour += texture(radianceMap, sampleVector).rgb * cos(theta) * sin(theta);
            index += 1.0;
        }
    }

	out_color = vec4((PI * sampledColour / index), 1.0);
}
