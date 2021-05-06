#version 450

#define PI 3.14159265359

layout (location = 0) in vec2 var_position;

layout (std140, binding = 0) uniform PrefilterData {
    int cube_face;
    float roughness;
};

layout (binding = 1) uniform samplerCube radianceMap;

layout (location = 0) out vec4 out_color;

float distributionGGX(vec3 n, vec3 h, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float nDotH = max(dot(n, h), 0.0);
    float nDotH2 = nDotH * nDotH;

    float nom   = a2;
    float denom = (nDotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

#if 0
float radicalInverse_VdC(uint bits) {
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i, uint N) {
	return vec2(float(i)/float(N), radicalInverse_VdC(i));
}
#endif

float vanDerCorpus(uint n, int base) {
    float invBase = 1.0 / float(base);
    float denom   = 1.0;
    float result  = 0.0;

    for(uint i = 0u; i < 32u; ++i) {
        if (n > 0) {
            denom   = mod(float(n), 2.0);
            result += denom * invBase;
            invBase = invBase / 2.0;
            n       = int(float(n) / 2.0);
        }
    }

    return result;
}

vec2 hammersley(uint i, uint n) {
    return vec2(float(i) / float(n), vanDerCorpus(i, 2));
}

vec3 importanceSampleGGX(vec2 xi, vec3 n, float roughness) {
	float a = roughness * roughness;

	float phi = 2.0 * PI * xi.x;
	float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 h;
	h.x = cos(phi) * sinTheta;
	h.y = sin(phi) * sinTheta;
	h.z = cosTheta;

	// from tangent-space H vector to world-space sample vector
	vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, n));
	vec3 bitangent = cross(n, tangent);

	vec3 sampleVec = tangent * h.x + bitangent * h.y + n * h.z;
	return normalize(sampleVec);
}

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
    vec3 N = normalize( vec3(var_position.x, -var_position.y, 1) );
    if(cube_face==2)
        N = normalize( vec3(var_position.x,  1, var_position.y) );
    else if(cube_face==3)
        N = normalize( vec3(var_position.x, -1,  -var_position.y) );
    else if(cube_face==0)
        N = normalize( vec3(  1, -var_position.y,-var_position.x) );
    else if(cube_face==1)
        N = normalize( vec3( -1, -var_position.y, var_position.x) );
    else if(cube_face==5)
        N = normalize( vec3(-var_position.x, -var_position.y, -1) );
#endif

    // make the simplyfying assumption that V equals R equals the normal
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;

    float resolution = 512.0; // resolution of source cubemap (per face)
    float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        vec2 Xi = hammersley(i, SAMPLE_COUNT);
        vec3 H = importanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D   = distributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float mipLevel = (roughness == 0.0) ? 0.0 : 0.5 * log2(saSample / saTexel);

            prefilteredColor += textureLod(radianceMap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    out_color = vec4(prefilteredColor / totalWeight, 1.0);
}
