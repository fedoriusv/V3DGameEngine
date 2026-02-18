#ifndef _SHADOW_COMMON_HLSL_
#define _SHADOW_COMMON_HLSL_

#include "global.hlsli"

#ifndef SHADOWMAP_FAST_COMPUTATION
#define SHADOWMAP_FAST_COMPUTATION 0
#endif

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

float GatherSample(Texture2DArray Shadowmap, SamplerState Sampler, in float3 Coord)
{
    float4 value = float4(
        Shadowmap.Sample(Sampler, Coord, int2(0, 0)).r,
        Shadowmap.Sample(Sampler, Coord, int2(1, 0)).r,
        Shadowmap.Sample(Sampler, Coord, int2(0, 1)).r,
        Shadowmap.Sample(Sampler, Coord, int2(1, 1)).r);
    
    return dot(value, 0.25.xxxx);
}

float shadow_linear_sample_PCF_1x1(Texture2DArray Shadowmap, SamplerState Sampler, in float2 Resolution, in float2 ClipNearFar, in float4 ShadowCoord, in uint Slice, in float Bias)
{
    if (is_inside_uv(ShadowCoord.xyz))
    {
        float kernelRadius = 2.0;
        float2 texelSize = rcp(Resolution);
        ShadowCoord.xy = clamp(ShadowCoord.xy, texelSize * kernelRadius, 1 - texelSize * kernelRadius);
        
        float dist = Shadowmap.Sample(Sampler, float3(ShadowCoord.xy, (float) Slice)).r;
        
        float linearDist = linearize_depth(dist, ClipNearFar.x, ClipNearFar.y);
        float linearDepth = linearize_depth(ShadowCoord.z, ClipNearFar.x, ClipNearFar.y);
        if (linearDist + Bias < linearDepth)
        {
            return 1.0;
        }
    }
    
    return 0.0;
}

float shadow_linear_sample_PCF_9x9(Texture2DArray Shadowmap, SamplerState Sampler, in float2 Resolution, in float2 ClipNearFar, in float4 ShadowCoord, in uint Slice, in float Bias)
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
    float kernelRadius = 9;
    float2 texelSize = rcp(Resolution);
    ShadowCoord.xy = clamp(ShadowCoord.xy, texelSize * kernelRadius, 1 - texelSize * kernelRadius);
    if (is_inside_uv(ShadowCoord.xyz)) 
    {
        [unroll(kernelSize)]
        for (uint i = 0; i < kernelSize; ++i)
        {
            float2 offset = kernel[i] * texelSize * 0.5;
            
            float dist = Shadowmap.Sample(Sampler, float3(ShadowCoord.xy + offset, (float) Slice)).r;
            float linearDist = linearize_depth(dist, ClipNearFar.x, ClipNearFar.y);
            float linearDepth = linearize_depth(ShadowCoord.z, ClipNearFar.x, ClipNearFar.y);
            if (linearDist + Bias < linearDepth)
            {
                shadow += 1.0;
            }
        }
    }
    
    return saturate(shadow / (float) kernelSize);
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

        float2 offset = kernel[i] * texelSize * ScaleFactor;
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
        float2 offset = kernel[i] * texelSize * ScaleFactor;
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

#endif //_SHADOW_COMMON_HLSL_