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
[[vk::binding(6, 1)]] Texture2D texture3 : register(t3, space1);
[[vk::binding(7, 1)]] RWTexture2DArray<uint> RWTexture0 : register(u0, space1);

///////////////////////////////////////////////////////////////////////////////////////

uint FloatToUintDepth(float depth)
{
    return asuint(depth);
}

float UintToFloatDepth(uint u)
{
    return asfloat(u);
}

uint WangHash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

// Get a float in [0, 1)
float RandomFloat(uint2 pixelCoord, uint frameIndex)
{
    uint hash = WangHash(pixelCoord.x + 1023 * pixelCoord.y + 7919 * frameIndex);
    return (hash & 0x00FFFFFF) / 16777216.0;
}

// Sample index for S samples
uint RandomSampleIndex(uint2 pixelCoord, uint frameIndex, uint S)
{
    uint hash = WangHash(pixelCoord.x * 73856093 ^ pixelCoord.y * 19349663 ^ frameIndex * 83492791);
    return hash % S;
}

uint RandomIndex(uint2 pixelCoord, uint frameSeed, uint S)
{
    uint hash = pixelCoord.x + 92837111 * pixelCoord.y + 689287499 * frameSeed;
    return hash % S;
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

//pass 1
[[vk::location(0)]] float transparency_total_alpha_pass1_ps(PS_GBUFFER_STANDARD_INPUT input, bool frontFace : SV_ISFRONTFACE) : SV_TARGET0
{
    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(input, CB_Viewport, CB_Model, texture0, texture1, texture2, samplerState);
    
    float3 color = GBubfferStruct.BaseColor.rgb;
    float opacity = GBubfferStruct.BaseColor.a;
    float emissive = 0.0;
    
    float depth = input.Position.z;
    uint2 pixelCoord = input.Position.xy;
    uint sampleCount = 4;
    
    uint samplemask = 0x0;
    if (opacity < 1.0)
    {
        float3 viewPos = mul(CB_Viewport.viewMatrix, float4(input.WorldPos, 1.0)).xyz;
        float propability = max(0.5, opacity);
        samplemask = a2c(propability, int2(input.Position.xy), viewPos, int(CB_Viewport.time), frontFace);
    }
    else
    {
        samplemask = 0xF; //all bits are enabled
    }
    
    for (int s = 0; s < sampleCount; ++s)
    {
        if (samplemask & (1 << s))
        {
            uint3 writeCoord = uint3(pixelCoord, s);
            uint depthSample = RWTexture0[writeCoord.xyz];
            uint currnetDepth = FloatToUintDepth(depth);
            InterlockedMax(RWTexture0[writeCoord.xyz], currnetDepth);
        }
    }

    return opacity;
}

///////////////////////////////////////////////////////////////////////////////////////

////pass 2
//void transparency_depth_pass2_ps(PS_GBUFFER_STANDARD_INPUT input, bool frontFace : SV_ISFRONTFACE, uint PrimitiveID : SV_PRIMITIVEID)
//{
//    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(input, CB_Viewport, CB_Model, texture0, texture1, texture2, samplerState);
    
//    float3 color = GBubfferStruct.BaseColor.rgb;
//    float opacity = GBubfferStruct.BaseColor.a;
//    float emissive = 0.0;
    
//    float depth = input.Position.z;
//    uint2 pixelCoord = input.Position.xy;
//    uint sampleCount = 4;
    

//    uint samplemask = 0x0;
//    if (opacity < 1.0)
//    {
//        float3 viewPos = mul(CB_Viewport.viewMatrix, float4(input.WorldPos, 1.0)).xyz;
//        samplemask = a2c(max(0.5, opacity), int2(input.Position.xy), viewPos, int(CB_Viewport.time), frontFace);
//    }
//    else
//    {
//        samplemask = 0xF; //all bits are enabled
//    }

//    //float2 positionScreenUV = input.Position.xy * (1.0 / CB_Viewport.viewportSize.xy);
//    //for (int s = 0; s < sampleCount; ++s)
//    //{
//    //    float2 offset = CB_Viewport.random.xy * (PrimitiveID + 1) * (s + 1);
//    //    float noise = texture3.Sample(samplerState, positionScreenUV + offset).a;
//    //    if (opacity >= noise)
//    //    {
//    //        samplemask |= 1 << s;
//    //    }
//    //}
    
//    for (int s = 0; s < sampleCount; ++s)
//    {
//        if (samplemask & (1 << s))
//        {
//            uint3 writeCoord = uint3(pixelCoord, s);
//            uint depthSample = RWTexture0[writeCoord.xyz];
//            uint currnetDepth = FloatToUintDepth(depth);
//            InterlockedMax(RWTexture0[writeCoord.xyz], currnetDepth); //reversed depth test
//        }
//    }
//}

///////////////////////////////////////////////////////////////////////////////////////

struct PS_TRANSPARENCY_OUTPUT
{
    [[vk::location(0)]] float4 Accumulate : COLOR;
    [[vk::location(1)]] float4 Material   : MATERIAL;
    [[vk::location(2)]] float2 Velocity   : VELOCITY;
};

//pass 3
PS_TRANSPARENCY_OUTPUT transparency_accumulate_color_pass3_ps(PS_GBUFFER_STANDARD_INPUT input)
{
    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(input, CB_Viewport, CB_Model, texture0, texture1, texture2, samplerState);
    
    float3 color = GBubfferStruct.BaseColor.rgb;
    float opacity = GBubfferStruct.BaseColor.a;
    float emissive = 0.0;
    
    uint sampleCount = 4;
    float depth = input.Position.z;
    uint2 pixelCoord = input.Position.xy;
    
    float visibility = 0.0;
    {
        int visibleSamples = 0;
        for (int i = 0; i < sampleCount; ++i)
        {
            uint3 readCoord = uint3(pixelCoord, i);
            uint depthSample = RWTexture0[readCoord.xyz];
            float depthInterval = FloatToUintDepth(depth);
            if (depthInterval <= depthSample) //depth test
            {
                ++visibleSamples;
            }
        }
        
        visibility = visibleSamples / sampleCount;
        visibility = clamp(visibility, 0.05, 1.0);
    }

    PS_TRANSPARENCY_OUTPUT Output;
    Output.Accumulate = float4(color * opacity * visibility, opacity * visibility);
    Output.Material = GBubfferStruct.Material;
    Output.Velocity = GBubfferStruct.Velocity;
    
    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

//pass 4
[[vk::location(0)]] float4 transparency_final_ps(PS_OFFSCREEN_INPUT input) : SV_TARGET0
{
    float3 baseColor = texture0.SampleLevel(samplerState, input.UV, 0).rgb;
    float4 accumulatedTransparencyColor = texture1.SampleLevel(samplerState, input.UV, 0);
    
    float totalAlpha = texture2.SampleLevel(samplerState, input.UV, 0).r;
    float alphaCorrected = saturate(accumulatedTransparencyColor.a / totalAlpha);
    
    float3 transparencyColor = accumulatedTransparencyColor.rgb;
    transparencyColor.rgb /= max(1.0, accumulatedTransparencyColor.a);
    transparencyColor.rgb *= alphaCorrected;
    
    float3 color = baseColor.rgb * (1.0 - alphaCorrected) + transparencyColor.rgb;
    return float4(color, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////

