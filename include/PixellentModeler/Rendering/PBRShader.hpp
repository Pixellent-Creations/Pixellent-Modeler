#pragma once

#include <string>

namespace PixellentModeler {

// PBR Shader - Cook-Torrance with normal mapping
static const char* kPBRVertexSrc = R"glsl(
#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 uv;
    mat3 TBN;
} vs_out;

void main() {
    vs_out.fragPos = vec3(uModel * vec4(aPosition, 1.0));
    vs_out.normal = normalize(vec3(uModel * vec4(aNormal, 0.0)));
    vs_out.uv = aUV;

    // Compute TBN matrix for normal mapping
    vec3 T = normalize(vec3(uModel * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(uModel * vec4(aBitangent, 0.0)));
    vec3 N = vs_out.normal;
    vs_out.TBN = mat3(T, B, N);

    gl_Position = uProjection * uView * vec4(vs_out.fragPos, 1.0);
}
)glsl";

static const char* kPBRFragmentSrc = R"glsl(
#version 330 core

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 uv;
    mat3 TBN;
} fs_in;

uniform vec3 uAlbedo;
uniform float uMetallic;
uniform float uRoughness;
uniform float uAO;

uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uAOMap;

uniform bool uHasAlbedoMap;
uniform bool uHasNormalMap;
uniform bool uHasMetallicMap;
uniform bool uHasRoughnessMap;
uniform bool uHasAOMap;

uniform vec3 uViewPos;
uniform vec3 uAmbientLight;

const int MAX_LIGHTS = 8;
struct Light {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    int type; // 0 = directional, 1 = point
};

uniform Light uLights[MAX_LIGHTS];
uniform int uLightCount;

const float PI = 3.14159265359;

// Cook-Torrance BRDF functions
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 normal = fs_in.normal;

    // Sample normal map if present
    if (uHasNormalMap) {
        normal = texture(uNormalMap, fs_in.uv).rgb;
        normal = normalize(normal * 2.0 - 1.0); // Convert [0,1] to [-1,1]
        normal = normalize(fs_in.TBN * normal);
    }

    vec3 albedo = uAlbedo;
    if (uHasAlbedoMap) {
        albedo = texture(uAlbedoMap, fs_in.uv).rgb;
    }

    float metallic = uMetallic;
    if (uHasMetallicMap) {
        metallic = texture(uMetallicMap, fs_in.uv).r;
    }

    float roughness = uRoughness;
    if (uHasRoughnessMap) {
        roughness = texture(uRoughnessMap, fs_in.uv).r;
    }

    float ao = uAO;
    if (uHasAOMap) {
        ao = texture(uAOMap, fs_in.uv).r;
    }

    vec3 N = normalize(normal);
    vec3 V = normalize(uViewPos - fs_in.fragPos);

    // F0 depends on metallic
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < uLightCount; ++i) {
        vec3 lightDir;
        float attenuation = 1.0;

        if (uLights[i].type == 0) { // Directional
            lightDir = normalize(-uLights[i].direction);
        } else { // Point
            lightDir = normalize(uLights[i].position - fs_in.fragPos);
            float distance = length(uLights[i].position - fs_in.fragPos);
            attenuation = 1.0 / (distance * distance);
        }

        vec3 H = normalize(V + lightDir);
        float distance = length(uLights[i].position - fs_in.fragPos);
        vec3 radiance = uLights[i].color * uLights[i].intensity * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, lightDir, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, lightDir), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        float NdotL = max(dot(N, lightDir), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = uAmbientLight * albedo * ao;
    vec3 color = ambient + Lo;

    // Tone mapping
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2)); // Gamma correction

    gl_FragColor = vec4(color, 1.0);
}
)glsl";

} // namespace PixellentModeler
