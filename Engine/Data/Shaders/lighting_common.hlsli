#ifndef _LIGTING_COMMON_HLSL_
#define _LIGTING_COMMON_HLSL_

#include "viewport.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

struct LightBuffer
{
    float4 position;
    float4 direction;
    float4 color;
    float  type;
    float  attenuation;
    float  intensity;
    float  temperature;
    
};

///////////////////////////////////////////////////////////////////////////////////////

float4 lambert_lighting(
    in float3 LightDir,
    in float3 WorldNormal,
    in float3 LightColor,
    in float3 PixelColor)
{
    float3 N = normalize(WorldNormal);
    float3 L = normalize(-LightDir);
    
    float diffuseKoeff = saturate(dot(N, L));
    return float4(diffuseKoeff * LightColor * PixelColor, 1.0);
}


///////////////////////////////////////////////////////////////////////////////////////

static const float PI = 3.14159265;

// Normal Distribution Function (GGX)
float D_GGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom + 1e-5);
}

// Geometry Function (Schlick-GGX approximation)
float G_Schlick_GGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // for direct lighting
    return NdotV / (NdotV * (1.0 - k) + k + 1e-5);
}

float G_Smith(float NdotV, float NdotL, float roughness)
{
    return G_Schlick_GGX(NdotV, roughness) * G_Schlick_GGX(NdotL, roughness);
}

// Fresnel Schlick approximation
float3 Fresnel_Schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

///////////////////////////////////////////////////////////////////////////////////////

// Cook-Torrance BRDF
float4 cook_torrance_BRDF(
    in uniform ConstantBuffer<Viewport> Viewport,
    in uniform ConstantBuffer<LightBuffer> Light,
    in float3 WorldPos,
    in float3 Albedo,
    in float3 Normals,
    in float Metallic,
    in float Roughness,
    in float Depth)
{
    // Build TBN matrix
    float3 N = normalize(Normals);
    float3 V = normalize(Viewport.cameraPosition.xyz - WorldPos);
    float3 L = normalize(-Light.direction.xyz);
    float3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    // Fresnel reflectance at normal incidence
    float3 F0 = float3(0.04, 0.04, 0.04); // dielectric
    F0 = lerp(F0, Albedo, Metallic); // metal uses albedo

    // BRDF components
    float D = D_GGX(NdotH, Roughness);
    float G = G_Smith(NdotV, NdotL, Roughness);
    float3 F = Fresnel_Schlick(VdotH, F0);
    
    float3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 1e-5;
    float3 specular = numerator / denominator;

    float3 kD = (1.0 - F) * (1.0 - Metallic);
    float3 diffuse = kD * Albedo / PI;
    
    float3 Lo = (diffuse + specular) * Light.color.rgb * NdotL;

    return float4(Lo, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_LIGTING_COMMON_HLSL_