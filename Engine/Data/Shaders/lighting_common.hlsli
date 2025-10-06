#ifndef _LIGTING_COMMON_HLSL_
#define _LIGTING_COMMON_HLSL_

#include "viewport.hlsli"
#include "environment_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

struct LightBuffer
{
    float4 position;
    float4 direction_range;
    float4 color;
    float  type;
    float  attenuation;
    float  intensity;
    float  temperature;
    
};

///////////////////////////////////////////////////////////////////////////////////////

static const float PI = 3.14159265;

///////////////////////////////////////////////////////////////////////////////////////

float3 temperature_RGB(float temperature)
{
    // Clamp to range
    temperature = clamp(temperature, 1000.0f, 40000.0f) / 100.0f;

    float3 rgb;

    // Red
    rgb.r = (temperature <= 66) ? 1.0 :
            clamp(1.292936186062745f * pow(temperature - 60.0f, -0.1332047592f), 0.0f, 1.0f);

    // Green
    rgb.g = (temperature <= 66) ?
            clamp(0.3900815787690196f * log(temperature) - 0.6318414437886275f, 0.0f, 1.0f) :
            clamp(1.129890860895294f * pow(temperature - 60.0f, -0.0755148492f), 0.0f, 1.0f);

    // Blue
    rgb.b = (temperature >= 66) ? 1.0 :
            (temperature <= 19) ? 0.0 :
            clamp(0.5432067891101961f * log(temperature - 10.0f) - 1.19625408914f, 0.0f, 1.0f);

    return rgb;
}

///////////////////////////////////////////////////////////////////////////////////////

float4 lambert_lighting(
    in uniform ConstantBuffer<Viewport> Viewport,
    in uniform ConstantBuffer<LightBuffer> Light,
    in float3 WorldPos,
    in float3 Direction,
    in float3 Albedo,
    in float3 Normals,
    in float Roughness,
    in float Metallic,
    in float Depth)
{
    float3 N = normalize(Normals);
    float3 L = -normalize(Direction);

    float diffuseKoeff = 1.0; //;max(dot(N, L), 0.01);
    return float4(diffuseKoeff * Albedo * Light.color.rgb, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////

float4 phong_blinn_lighting(
    in uniform ConstantBuffer<Viewport> Viewport,
    in uniform ConstantBuffer<LightBuffer> Light,
    in float3 WorldPos,
    in float3 Direction,
    in float3 Albedo,
    in float3 Specular,
    in float3 Normals,
    in float Roughness,
    in float Metallic,
    in float Depth)
{
    float3 N = normalize(Normals);
    float3 L = -normalize(Direction);
    float3 V = normalize(Viewport.cameraPosition.xyz - WorldPos);
    float3 H = normalize(L + V);
    
    float diffuseKoeff = max(dot(N, L), 0.01);
    float4 color = float4(diffuseKoeff * Albedo * Light.color.rgb, 1.0);
    if (diffuseKoeff > 0.01)
    {
        color.rgb += Specular * pow(max(dot(N, H), 0.0), Light.intensity) * Light.attenuation;
        
        //float R = reflect(-L, N);
        //color.rgb += Specular * pow(max(dot(R, V), 0.0), Light.intensity) * Light.attenuation;
    }

    return color;
}

///////////////////////////////////////////////////////////////////////////////////////

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
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

///////////////////////////////////////////////////////////////////////////////////////

// Cook-Torrance BRDF
float4 cook_torrance_BRDF(
    in uniform ConstantBuffer<Viewport> Viewport,
    in uniform ConstantBuffer<LightBuffer> Light,
    in EnvironmentBuffer Environment,
    in float3 WorldPos,
    in float3 Direction,
    in float3 Albedo,
    in float3 Normals,
    in float Roughness,
    in float Metallic,
    in float Depth)
{
    // Build TBN matrix
    float3 N = normalize(Normals);
    float3 V = normalize(Viewport.cameraPosition.xyz - WorldPos);
    float3 L = -normalize(Direction);
    float3 H = normalize(L + V);

    float NdotV = abs(dot(N, V)) + 1e-5;
    float NdotL = max(dot(N, L), 0.01);
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(max(dot(V, H), 0.0));

    // Fresnel reflectance at normal incidence
    float3 F0 = float3(0.04, 0.04, 0.04); // dielectric
    F0 = lerp(F0, Albedo, Metallic); // metal uses albedo
    
    // BRDF components
    float D = D_GGX(NdotH, Roughness);
    float G = G_Smith(NdotV, NdotL, Roughness);
    float3 F = Fresnel_Schlick(VdotH, F0);
    
    float3 numerator = D * G * F;
    float denominator = max(4.0 * NdotV * NdotL, 0.001);
    float3 specular = numerator / denominator;

    float3 kD = (1.0 - F) * (1.0 - Metallic);
    float3 diffuse = kD * Albedo / PI;
    
    //temp
    float constant = 1.0f;
    float lin = 0.09f;
    float quadratic = 0.032f;
    float distance = length(Direction);
    float attenuation = 1.0 / (constant + lin * distance + quadratic * (distance * distance));
    
    float3 lightColor = temperature_RGB(Light.temperature) * Light.color.rgb * Light.intensity;
    float3 radiance = lightColor * attenuation;
    float3 Lo = (diffuse + specular) * radiance * NdotL;

    return float4(Lo, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_LIGTING_COMMON_HLSL_