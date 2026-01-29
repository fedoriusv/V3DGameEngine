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
[[vk::binding(2, 1)]] SamplerState s_LinearMirrorSampler       : register(s0, space1);
[[vk::binding(3, 1)]] SamplerState s_LinearClampSampler        : register(s1, space1);
[[vk::binding(4, 1)]] Texture2D t_ColorTexture                 : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D t_LUTTexture                   : register(t2, space1);

///////////////////////////////////////////////////////////////////////////////////////

float3 tonemap_Reinhard(in float3 color)
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

float3 lut_2D(float3 color, float lutSize)
{
    color = saturate(color);
    if (cb_Tonemapper.lut == 0)
    {
        return color;
    }
    
    float2 texelsize = 1.0 / lutSize;
    texelsize.x /= lutSize;

    float3 lutcoord = float3((color.xy * lutSize - color.xy + 0.5) * texelsize.xy, color.z * lutSize - color.z);
    const float lerpfact = frac(lutcoord.z);
    
    //float blue = color.b * (lutSize - 1.0);
    //float slice0 = floor(blue);
    //lutcoord.x += slice0 * texelsize.y;
    lutcoord.x += (lutcoord.z - lerpfact) * texelsize.y;
    
    float3 c0 = t_LUTTexture.Sample(s_LinearMirrorSampler, lutcoord.xy).rgb;
    float3 c1 = t_LUTTexture.Sample(s_LinearMirrorSampler, float2(lutcoord.x + texelsize.y, lutcoord.y)).rgb;
    float3 lutcolor = lerp(c0, c1, lerpfact);
    
    //TODO fix r 
    //lutcolor.r = color.r;
    
    float fLUT_AmountChroma = 1.0;
    float fLUT_AmountLuma = 1.0;
    return lerp(normalize(color), normalize(lutcolor), fLUT_AmountChroma) * lerp(length(color), length(lutcolor), fLUT_AmountLuma);
}

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 tonemapping_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float4 hdrColor = t_ColorTexture.SampleLevel(s_LinearMirrorSampler, Input.UV, 0);
    float exposure = cb_Tonemapper.exposure;
    
    if (cb_Tonemapper.tonemapper == 1) //Reinhard
    {
        hdrColor.rgb = tonemap_Reinhard(hdrColor.rgb * exposure);
    }
    else if (cb_Tonemapper.tonemapper == 2) //ACES
    {
        hdrColor.rgb = tonemap_ACES_approx(hdrColor.rgb * exposure);
    }
    else if (cb_Tonemapper.tonemapper == 3) //Khronos PBR Neutral
    {
        hdrColor.rgb = tonemap_KhronosPBR_Neutral(hdrColor.rgb * exposure);
    }
    
    //TODO
    float with, height;
    t_LUTTexture.GetDimensions(with, height);
    hdrColor.rgb = lut_2D(hdrColor.rgb, height);
    
    // Gamma correction (linear -> sRGB)
    float gamma = cb_Tonemapper.gamma;
    float4 ldrColor = float4(pow(hdrColor.rgb * exposure, 1.0 / gamma), 1.0);

    return ldrColor;
}

///////////////////////////////////////////////////////////////////////////////////////
