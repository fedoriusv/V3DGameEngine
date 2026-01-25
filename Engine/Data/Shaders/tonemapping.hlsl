#include "global.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

struct Tonemapper
{
    uint tonemapper;
    uint lut;
    float exposure;
    float gamma;
};

[[vk::binding(1, 1)]] ConstantBuffer<Tonemapper> cb_Tonemapper : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState s_ColorSampler              : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D t_ColorTexture                 : register(t1, space1);

///////////////////////////////////////////////////////////////////////////////////////

float3 tonemap_reinhard(in float3 color)
{
    return color / (1.0 + color);
}

//ACES Filmic https://dev.epicgames.com/documentation/en-us/unreal-engine/color-grading-and-the-filmic-tonemapper-in-unreal-engine
float3 tonemap_ACES_approx(in float3 color)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;

    return (color * (a * color + b)) / (color * (c * color + d) + e);
}

float3 tonemap_KhronosPBR_Neutral(in float3 color)
{
    const float startCompression = 0.8;
    const float desaturation = 0.15;

    float x = min(color.r, min(color.g, color.b));
    float offset = (x < 0.08) ? x - 6.25 * x * x : 0.04;

    color -= offset;

    float peak = max(color.r, max(color.g, color.b));

    if (peak < startCompression)
        return color;

    float d = 1.0 - startCompression;
    float newPeak = 1.0 - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);
    return lerp(color, newPeak.xxx, g);
}

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 tonemapping_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float4 hdrColor = t_ColorTexture.SampleLevel(s_ColorSampler, Input.UV, 0);
    float exposure = cb_Tonemapper.exposure;
    
    if (cb_Tonemapper.tonemapper == 1) //Reinhard
    {
        hdrColor.rgb = tonemap_reinhard(hdrColor.rgb * exposure);
    }
    else if (cb_Tonemapper.tonemapper == 2) //ACES
    {
        hdrColor.rgb = tonemap_ACES_approx(hdrColor.rgb * exposure);
    }
    else if (cb_Tonemapper.tonemapper == 3) //Khronos PBR Neutral
    {
        hdrColor.rgb = tonemap_KhronosPBR_Neutral(hdrColor.rgb * exposure);
    }
    
    // Gamma correction (linear -> sRGB)
    float gamma = cb_Tonemapper.gamma;
    float4 ldrColor = float4(pow(hdrColor.rgb * exposure, 1.0 / gamma), 1.0);

    return ldrColor;
}

///////////////////////////////////////////////////////////////////////////////////////
