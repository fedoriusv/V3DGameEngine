#include "global.hlsli"
#include "viewport.hlsli"
#include "gbuffer_common.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> CB_Model : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D texture0 : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D texture1 : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D texture2 : register(t2, space1);
[[vk::binding(7, 1)]] RWStructuredBuffer<uint4> RWBuffer0 : register(u0, space1);

///////////////////////////////////////////////////////////////////////////////////////

uint FloatToUint(float value)
{
    return asuint(value);
}

float UintToFloat(uint u)
{
    return asfloat(u);
}

float dither2x2(int2 pixel)
{
    float4 pattern = float4(0.25, 0.75, 0.75, 0.25);
    return pattern[pixel.x % 2 + (pixel.y % 2) * 2];
}

float hash2d(float2 seed)
{
    return frac(sin(dot(seed, float2(12.9898, 78.233))) * 43758.5453);
}

float hash4d(float4 seed)
{
    return hash2d(float2(hash2d(float2(hash2d(seed.xy), seed.z)), seed.w));
}

// Modified version of Inigo Quilez' function for generating per-fragment alpha-to-coverage sample
// masks, https://twitter.com/iquilezles/status/947717661496041472. Here we use a simple 4D hash to
// randomize the sample mask and rely on TAA to suppress the noise.
int a2c(float alpha, int2 pixel, float3 view_position, int frame_id, bool frontFace)
{
    const int msaa = 4;

    // Dither the input alpha value with a 2x2 checker pattern to increase the number of alpha
    // levels from 5 to 9
    float dither = dither2x2(pixel + int2(int(frontFace), frame_id % 2));
    alpha = clamp(max(0.125, alpha) + (dither - 0.5) / float(msaa), 0.0, 1.0);

    // Compute alpha-to-coverage sample mask
    int mask = ~(0xff << int(alpha * float(msaa) + 0.5)) & 0xf;

    // Randomize the mask
    float rnd = hash4d(float4(view_position, frame_id % 1000));
    int shift = int(msaa * rnd) % msaa;
    mask = (((mask << 4) | mask) >> shift) & 0xf; // barrel shift

    return mask;
}

///////////////////////////////////////////////////////////////////////////////////////

float msoit_mrt_ps(PS_GBUFFER_STANDARD_INPUT input, bool frontFace : SV_ISFRONTFACE, uint PrimitiveID : SV_PRIMITIVEID) : SV_TARGET0
{
    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(input, CB_Viewport, CB_Model, texture0, texture1, texture2, samplerState);
    
    float3 color = GBubfferStruct.BaseColor.rgb;
    float opacity = GBubfferStruct.BaseColor.a;
    float emissive = 0.0;
    
    float3 viewPos = mul(CB_Viewport.viewMatrix, float4(input.WorldPos, 1.0)).xyz;
    int samplemask = 15;//    a2c(opacity, int2(input.Position.xy), viewPos, int(CB_Viewport.time), frontFace);
    
    uint2 pixelCoord = input.Position.xy;
    uint sampleCount = 4;
    for (int s = 0; s < sampleCount; ++s)
    {
        if (samplemask & (1 << s))
        {
            float3 colorSample = color.rgb * opacity;
            uint index = s * CB_Viewport.viewportSize.x * CB_Viewport.viewportSize.y + pixelCoord.y * CB_Viewport.viewportSize.x + pixelCoord.x;
            
            InterlockedAdd(RWBuffer0[index].r, FloatToUint(0.25));
            InterlockedAdd(RWBuffer0[index].a, FloatToUint(0.25));
            //InterlockedAdd(RWBuffer0[index].r, FloatToUint(colorSample.r));
            //InterlockedAdd(RWBuffer0[index].g, FloatToUint(colorSample.g));
            //InterlockedAdd(RWBuffer0[index].b, FloatToUint(colorSample.b));
            //InterlockedAdd(RWBuffer0[index].a, FloatToUint(opacity));
        }
    }
    
    return opacity;
}

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 msoit_resolve_ps(PS_OFFSCREEN_INPUT input) : SV_TARGET0
{
    float3 baseColor = texture0.SampleLevel(samplerState, input.UV, 0).rgb;
    float totalAlpha = texture1.SampleLevel(samplerState, input.UV, 0).r;
    
    uint2 pixelCoord = input.Position.xy;

    float4 accumulatedTransparencyColor = float4(0.0);
    uint sampleCount = 4;
    for (int s = 0; s < sampleCount; ++s)
    {
        uint index = s * CB_Viewport.viewportSize.x * CB_Viewport.viewportSize.y + pixelCoord.y * CB_Viewport.viewportSize.x + pixelCoord.x;
        uint4 colorSample = RWBuffer0[index].rgba;
        accumulatedTransparencyColor += float4(UintToFloat(colorSample.r), UintToFloat(colorSample.g), UintToFloat(colorSample.b), UintToFloat(colorSample.a));
    }
    accumulatedTransparencyColor /= sampleCount;

    float alphaCorrected = saturate(accumulatedTransparencyColor.a / totalAlpha);
    
    float3 transparencyColor = accumulatedTransparencyColor.rgb;
    transparencyColor.rgb /= max(1.0, accumulatedTransparencyColor.a);
    transparencyColor.rgb *= alphaCorrected;
    
    float3 color = baseColor.rgb * (1.0 - alphaCorrected) + transparencyColor.rgb;
    return float4(color, 1.0);

}

///////////////////////////////////////////////////////////////////////////////////////