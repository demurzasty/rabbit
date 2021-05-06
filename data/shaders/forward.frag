#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PI 3.14159265359

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_texcoord;
layout (location = 2) in vec3 v_normal;

layout (std140, binding = 0) uniform CameraData {
    mat4 proj;
    mat4 view;
    vec3 u_camera_position;
};

layout (std140, binding = 2) uniform MaterialData {
    vec3 u_base_color;
    float u_roughness;
    float u_metallic;
};

layout(binding = 3) uniform sampler2D u_albedo_map;
layout(binding = 4) uniform samplerCube u_radiance_map;
layout(binding = 5) uniform samplerCube u_irradiance_map;
layout(binding = 6) uniform samplerCube u_prefilter_map;
layout(binding = 7) uniform sampler2D u_brdf_map;

layout(binding = 8) uniform sampler2D u_normal_map;
layout(binding = 9) uniform sampler2D u_roughness_map;
layout(binding = 10) uniform sampler2D u_metallic_map;
layout(binding = 11) uniform sampler2D u_emissive_map;

layout (location = 0) out vec4 out_color;

float distribution_ggx(vec3 n, vec3 h, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float n_dot_h = max(dot(n, h), 0.0);
    float n_dot_h2 = n_dot_h * n_dot_h;

    float nom   = a2;
    float denom = (n_dot_h2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float geometry_schlick_ggx(float n_dot_v, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = n_dot_v;
    float denom = n_dot_v * (1.0 - k) + k;

    return nom / denom;
}

float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness) {

    float n_dot_v = max(dot(n, v), 0.0);
    float n_dot_l = max(dot(n, l), 0.0);
    float ggx2 = geometry_schlick_ggx(n_dot_v, roughness);
    float ggx1 = geometry_schlick_ggx(n_dot_l, roughness);

    return ggx1 * ggx2;
}

vec3 fresnel_schlick(float cos_theta, vec3 f0) {
    return f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
}

vec3 fresnel_schlick_roughness(float cos_theta, vec3 f0, float roughness) {
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - cos_theta, 5.0);
}

mat3 cotangent_frame(vec3 n, vec3 p, vec2 uv) {
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    // solve the linear system
    vec3 dp2perp = cross(dp2, n);
    vec3 dp1perp = cross(n, dp1);
    vec3 t = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 b = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame
    float invmax = 1.0 / sqrt(max(dot(t, t), dot(b, b)));
    return mat3(t * invmax, b * invmax, n);
}

vec3 perturb(vec3 map, vec3 n, vec3 v, vec2 texcoord) {
  mat3 tbn = cotangent_frame(n, -v, texcoord);
  return normalize(tbn * map);
}

void main() {
    vec3 albedo = u_base_color * texture(u_albedo_map, v_texcoord).rgb;
    float roughness = u_roughness * texture(u_roughness_map, v_texcoord).r;
    float metallic = u_metallic * texture(u_metallic_map, v_texcoord).r;

    vec3 v = normalize(u_camera_position - v_position);
    vec3 n = normalize(v_normal);

    n = perturb(texture(u_normal_map, v_texcoord).rgb * 2.0 - 1.0, n, v, v_texcoord);

    float n_dot_v = abs(dot(n, v)) + 1e-5;

    vec3 f0 = mix(vec3(0.04), albedo, metallic);

    vec3 lo = vec3(0.0);

    // TODO: Iterate through nearest lights.
    {
        // TODO: Light color and intensity.
        vec3 radiance = vec3(1.0);
        float intensity = 1.0;

        vec3 l = -normalize(vec3(-1.0, -1.0, -1.0));

        vec3 h = normalize(v + l);
        vec3 f = fresnel_schlick(max(dot(h, v), 0.0), f0);

        vec3 ks = f;
        vec3 kd = (1.0 - ks) * (1.0 - metallic);

        float ndf = distribution_ggx(n, h, roughness);
        float g = geometry_smith(n, v, l, roughness);

        float n_dot_l = max(dot(n, l), 0.0);

        vec3 nominator = ndf * g * f;
        float denominator = 4.0 * n_dot_v * n_dot_l + 0.001;
        vec3 specular = nominator / denominator;

        lo += (kd * albedo / PI + specular) * radiance * intensity * n_dot_l;
    }

    vec3 ks = fresnel_schlick_roughness(max(dot(n, v), 0.0), f0, roughness);
    vec3 kd = (1.0 - ks) * (1.0 - metallic);

    vec3 irradiance = texture(u_irradiance_map, n).rgb;
    vec3 diff = irradiance * albedo;

    vec3 reflected = reflect(-v, n);

    vec3 prefilter_color = textureLod(u_prefilter_map, reflected, roughness * 6.0).rgb;
    vec2 brdf = texture(u_brdf_map, vec2(n_dot_v, roughness)).rg;
    vec3 spec = prefilter_color * (ks * brdf.x + brdf.y);

    vec3 ambient = kd * diff + spec + texture(u_emissive_map, v_texcoord).rgb;

    out_color = vec4(ambient + lo, 1.0);
}
