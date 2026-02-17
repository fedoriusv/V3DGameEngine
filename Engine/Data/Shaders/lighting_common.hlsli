#ifndef _LIGTING_COMMON_HLSL_
#define _LIGTING_COMMON_HLSL_

#include "global.hlsli"
#include "viewport.hlsli"
#include "environment_common.hlsli"

#ifndef SHADOWMAP_FAST_COMPUTATION
#define SHADOWMAP_FAST_COMPUTATION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////

struct LightBuffer
{
    float3 position;
    float3 direction;
    float4 color;
    float4 attenuation;
    float  intensity;
    float  temperature;
};

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
    in float Distance,
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
        // Attenuation
        float kConstant = Light.attenuation.x;
        float kLinear = Light.attenuation.y;
        float kQuadratic = Light.attenuation.z;
        float attenuation = 1.0 / max(kConstant + kLinear * Distance + kQuadratic * (Distance * Distance), 1e-5);
        
        color.rgb += Specular * pow(max(dot(N, H), 0.0), Light.intensity) * attenuation;
        
        //float R = reflect(-L, N);
        //color.rgb += Specular * pow(max(dot(R, V), 0.0), Light.intensity) * attenuation;
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
    float k = (r * r) / 8.0; // UE4 style
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
    in Viewport Viewport,
    in LightBuffer Light,
    in EnvironmentBuffer Environment,
    in float3 WorldPos,
    in float Distance,
    in float3 Albedo,
    in float3 Normals,
    in float Roughness,
    in float Metallic,
    in float Depth,
    in float Shadow)
{
    // Build TBN matrix
    float3 N = normalize(Normals);
    float3 V = normalize(Viewport.cameraPosition.xyz - WorldPos);
    float3 L = -normalize(Light.direction.xyz);
    float3 H = normalize(L + V);

    float NdotV = abs(dot(N, V)) + 1e-5;
    float NdotL = max(dot(N, L), 1e-5);
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
    float denominator = max(4.0 * NdotV * NdotL, 1e-5);
    float3 specular = numerator / denominator;

    float3 kD = (1.0 - F) * (1.0 - Metallic);
    float3 diffuse = kD * Albedo / PI;
    
    // Attenuation
    float kConstant = Light.attenuation.x;
    float kLinear = Light.attenuation.y;
    float kQuadratic = Light.attenuation.z;
    float attenuation = 1.0 / max(kConstant + kLinear * Distance + kQuadratic * (Distance * Distance), 1e-5);

    // Smooth fade to zero near range 
    float rangeFactor = saturate(1.0 - (Distance / max(Light.attenuation.w, 0.01)));
    attenuation *= rangeFactor * rangeFactor; // sharper falloff near the end
    
    float3 lightColor = temperature_RGB(Light.temperature) * Light.color.rgb * Light.intensity;
    float3 radiance = lightColor * attenuation * Shadow;
    
    float3 Lo = (diffuse + specular) * radiance * NdotL;
    
    // Ambient / IBL (simple ambient approximation)
    float3 ao = float3(1.0, 1.0, 1.0);
    float3 ambient = float3(0.03, 0.03, 0.03) * Albedo * ao * radiance * Shadow;
    
    float4 color = float4(Lo + ambient, 1.0);
    return color;
}

///////////////////////////////////////////////////////////////////////////////////////

float2 parallax_mapping(in float Height, in float Bump, in float2 UV, in float3 View)
{
    float2 P = View.xy/* / View.z*/ * (Height/* * Bump*/);
    return UV + P;
}

///////////////////////////////////////////////////////////////////////////////////////

float shadow_projection_fast(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float4 ShadowCoord, in int2 Offset, in uint Slice)
{
    if (is_inside_uv(ShadowCoord.xyz))
    {
        return Shadowmap.SampleCmpLevelZero(CompareSampler, float3(ShadowCoord.xy, (float) Slice), ShadowCoord.z, Offset);
    }

    return 0.0;
}

float shadow_projection(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float4 ShadowCoord, in int2 Offset, in uint Slice)
{
    float2 stc = ShadowCoord.xy * Resolution + 0.5.xx;
    float2 tcs = floor(stc);
    ShadowCoord.xy = tcs / Resolution;

    if (is_inside_uv(ShadowCoord.xyz))
    {
        float4 dist = Shadowmap.GatherCmpRed(CompareSampler, float3(ShadowCoord.xy, (float) Slice), ShadowCoord.z, Offset);
        float2 fracShadow = stc - tcs;
        return lerp(lerp(dist.w, dist.z, fracShadow.x), lerp(dist.x, dist.y, fracShadow.x), fracShadow.y);
    }

    return 0.0;
}

float shadow_sample_PCF_1x1(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float4 ShadowCoord, in uint Slice)
{
#if SHADOWMAP_FAST_COMPUTATION
    float dist = shadow_projection_fast(Shadowmap, CompareSampler, Resolution, ShadowCoord, int2(0, 0), Slice);
#else
    float dist = shadow_projection(Shadowmap, CompareSampler, Resolution, ShadowCoord, int2(0, 0), Slice);
#endif
    if (depth_test_nonlinear(dist, ShadowCoord.z))
    {
        return 1.0;
    }
    
    return 0.0;
}

float shadow_linear_sample_PCF_1x1(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float2 ClipNearFar, in float4 ShadowCoord, in uint Slice)
{
#if SHADOWMAP_FAST_COMPUTATION
    float dist = shadow_projection_fast(Shadowmap, CompareSampler, Resolution, ShadowCoord, int2(0, 0), Slice);
#else
    float dist = shadow_projection(Shadowmap, CompareSampler, Resolution, ShadowCoord, int2(0, 0), Slice);
#endif
    float linearDist = linearize_depth(dist, ClipNearFar.x, ClipNearFar.y);
    float linearDepth = linearize_depth(ShadowCoord.z, ClipNearFar.x, ClipNearFar.y);
    if (linearDist < linearDepth)
    {
        return 1.0;
    }
    
    return 0.0;
}

float shadow_sample_PCF_3x3(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float2 ScaleFactor, in float4 ShadowCoord, in uint Slice)
{
    static const uint kernelSize = 9;
    static const float2 kernel[kernelSize] =
    {
        float2(-1.0, -1.0), float2(0.0, -1.0), float2(1.0, -1.0),
        float2(-1.0,  0.0), float2(0.0,  0.0), float2(1.0,  0.0),
        float2(-1.0,  1.0), float2(0.0,  1.0), float2(1.0,  1.0)
    };

    float shadow = 0.0;
    float2 texelSize = rcp(Resolution);
    
    [unroll(kernelSize)]
    for (uint i = 0; i < kernelSize; ++i)
    {

        float2 offset = kernel[i] * ScaleFactor;
#if SHADOWMAP_FAST_COMPUTATION
        float dist = shadow_projection_fast(Shadowmap, CompareSampler, Resolution, ShadowCoord + float4(offset.xy, 0.0, 0.0), int2(0, 0), Slice);
#else
        float dist = shadow_projection(Shadowmap, CompareSampler, Resolution, ShadowCoord + float4(offset.xy, 0.0, 0.0), int2(0, 0), Slice);
#endif
        if (depth_test_nonlinear(dist, ShadowCoord.z))
        {
            shadow += dist;
        }
    }
    
    return saturate(shadow / (float) kernelSize);
}

float shadow_sample_PCF_9x9(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float2 ScaleFactor, in float4 ShadowCoord, in uint Slice)
{
    static const uint kernelSize = 81;
    static const float2 kernel[kernelSize] =
    {
        float2(-4.0, -4.0), float2(-3.0, -4.0), float2(-2.0, -4.0), float2(-1.0, -4.0), float2(0.0, -4.0), float2(1.0, -4.0), float2(2.0, -4.0), float2(3.0, -4.0), float2(4.0, -4.0),
        float2(-4.0, -3.0), float2(-3.0, -3.0), float2(-2.0, -3.0), float2(-1.0, -3.0), float2(0.0, -3.0), float2(1.0, -3.0), float2(2.0, -3.0), float2(3.0, -3.0), float2(4.0, -3.0),
        float2(-4.0, -2.0), float2(-3.0, -2.0), float2(-2.0, -2.0), float2(-1.0, -2.0), float2(0.0, -2.0), float2(1.0, -2.0), float2(2.0, -2.0), float2(3.0, -2.0), float2(4.0, -2.0),
        float2(-4.0, -1.0), float2(-3.0, -1.0), float2(-2.0, -1.0), float2(-1.0, -1.0), float2(0.0, -1.0), float2(1.0, -1.0), float2(2.0, -1.0), float2(3.0, -1.0), float2(4.0, -1.0),
        float2(-4.0,  0.0), float2(-3.0,  0.0), float2(-2.0,  0.0), float2(-1.0,  0.0), float2(0.0,  0.0), float2(1.0,  0.0), float2(2.0,  0.0), float2(3.0,  0.0), float2(4.0,  0.0),
        float2(-4.0,  1.0), float2(-3.0,  1.0), float2(-2.0,  1.0), float2(-1.0,  1.0), float2(0.0,  1.0), float2(1.0,  1.0), float2(2.0,  1.0), float2(3.0,  1.0), float2(4.0,  1.0),
        float2(-4.0,  2.0), float2(-3.0,  2.0), float2(-2.0,  2.0), float2(-1.0,  2.0), float2(0.0,  2.0), float2(1.0,  2.0), float2(2.0,  2.0), float2(3.0,  2.0), float2(4.0,  2.0),
        float2(-4.0,  3.0), float2(-3.0,  3.0), float2(-2.0,  3.0), float2(-1.0,  3.0), float2(0.0,  3.0), float2(1.0,  3.0), float2(2.0,  3.0), float2(3.0,  3.0), float2(4.0,  3.0),
        float2(-4.0,  4.0), float2(-3.0,  4.0), float2(-2.0,  4.0), float2(-1.0,  4.0), float2(0.0,  4.0), float2(1.0,  4.0), float2(2.0,  4.0), float2(3.0,  4.0), float2(4.0,  4.0)
    };

    float shadow = 0.0;
    float2 texelSize = rcp(Resolution);
    
    [unroll(kernelSize)]
    for (uint i = 0; i < kernelSize; ++i)
    {
        float2 offset = kernel[i] * ScaleFactor;
#if SHADOWMAP_FAST_COMPUTATION
        float dist = shadow_projection_fast(Shadowmap, CompareSampler, Resolution, ShadowCoord + float4(offset.xy, 0.0, 0.0), int2(0, 0), Slice);
#else
        float dist = shadow_projection(Shadowmap, CompareSampler, Resolution, ShadowCoord + float4(offset.xy, 0.0, 0.0), int2(0, 0), Slice);
#endif
        if (depth_test_nonlinear(dist, ShadowCoord.z))
        {
            shadow += dist;
        }
    }

    return saturate(shadow / (float) kernelSize);
}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_LIGTING_COMMON_HLSL_