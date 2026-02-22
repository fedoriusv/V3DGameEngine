#ifndef _SHADOW_COMMON_HLSL_
#define _SHADOW_COMMON_HLSL_

#include "global.hlsli"

#ifndef SHADOWMAP_FAST_COMPUTATION
#define SHADOWMAP_FAST_COMPUTATION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////

float shadow_projection_fast(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float3 ShadowCoord, in int2 Offset, in uint Slice)
{
    if (_inside_uv(ShadowCoord.xyz))
    {
        return Shadowmap.SampleCmpLevelZero(CompareSampler, float3(ShadowCoord.xy, (float) Slice), ShadowCoord.z, Offset);
    }

    return 0.0;
}

float shadow_projection(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float3 ShadowCoord, in int2 Offset, in uint Slice)
{
    if (_inside_uv(ShadowCoord.xyz))
    {
        float2 stc = ShadowCoord.xy * Resolution + 0.5.xx;
        float2 tcs = floor(stc);
        ShadowCoord.xy = tcs / Resolution;
        
        float4 dist = Shadowmap.GatherCmpRed(CompareSampler, float3(ShadowCoord.xy, (float) Slice), ShadowCoord.z, Offset);
        float2 fracShadow = stc - tcs;
        return lerp(lerp(dist.w, dist.z, fracShadow.x), lerp(dist.x, dist.y, fracShadow.x), fracShadow.y);
    }

    return 0.0;
}

float shadow_sample(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float3 ShadowCoord, in uint Slice)
{
#if SHADOWMAP_FAST_COMPUTATION
    float dist = shadow_projection_fast(Shadowmap, CompareSampler, Resolution, ShadowCoord, int2(0, 0), Slice);
#else
    float dist = shadow_projection(Shadowmap, CompareSampler, Resolution, ShadowCoord, int2(0, 0), Slice);
#endif
    if (_nonlinear_depth_test(dist, ShadowCoord.z))
    {
        return 1.0;
    }
    
    return 0.0;
}

float shadow_sample_PCF_3x3(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float2 ScaleFactor, in float3 ShadowCoord, in uint Slice)
{
    float shadow = 0.0;
    float2 texelSize = rcp(Resolution);
    
    [unroll(g_Gaussian3x3_KernelSize)]
    for (uint i = 0; i < g_Gaussian3x3_KernelSize; ++i)
    {

        float2 offset = g_Gaussian3x3_Kernel[i] * texelSize * ScaleFactor;
#if SHADOWMAP_FAST_COMPUTATION
        float dist = shadow_projection_fast(Shadowmap, CompareSampler, Resolution, ShadowCoord + float4(offset.xy, 0.0, 0.0), int2(0, 0), Slice);
#else
        float dist = shadow_projection(Shadowmap, CompareSampler, Resolution, ShadowCoord + float3(offset.xy, 0.0), int2(0, 0), Slice);
#endif
        if (_nonlinear_depth_test(dist, ShadowCoord.z))
        {
            shadow += dist;
        }
    }
    
    return saturate(shadow / (float) g_Gaussian3x3_KernelSize);
}

float shadow_sample_PCF_5x5(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float2 ScaleFactor, in float3 ShadowCoord, in uint Slice)
{
    float shadow = 0.0;
    float2 texelSize = rcp(Resolution);
    
    [unroll(g_Gaussian5x5_KernelSize)]
    for (uint i = 0; i < g_Gaussian5x5_KernelSize; ++i)
    {
        float2 offset = g_Gaussian5x5_Kernel[i] * texelSize * ScaleFactor;
#if SHADOWMAP_FAST_COMPUTATION
        float dist = shadow_projection_fast(Shadowmap, CompareSampler, Resolution, ShadowCoord + float4(offset.xy, 0.0, 0.0), int2(0, 0), Slice);
#else
        float dist = shadow_projection(Shadowmap, CompareSampler, Resolution, ShadowCoord + float3(offset.xy, 0.0), int2(0, 0), Slice);
#endif
        if (_nonlinear_depth_test(dist, ShadowCoord.z))
        {
            shadow += dist;
        }
    }

    return saturate(shadow / (float) g_Gaussian5x5_KernelSize);
}

float shadow_sample_PCF_9x9(Texture2DArray Shadowmap, SamplerComparisonState CompareSampler, in float2 Resolution, in float2 ScaleFactor, in float3 ShadowCoord, in uint Slice)
{
    float shadow = 0.0;
    float2 texelSize = rcp(Resolution);
    
    [unroll(g_Gaussian9x9_KernelSize)]
    for (uint i = 0; i < g_Gaussian9x9_KernelSize; ++i)
    {
        float2 offset = g_Gaussian9x9_Kernel[i] * texelSize * ScaleFactor;
#if SHADOWMAP_FAST_COMPUTATION
        float dist = shadow_projection_fast(Shadowmap, CompareSampler, Resolution, ShadowCoord + float4(offset.xy, 0.0, 0.0), int2(0, 0), Slice);
#else
        float dist = shadow_projection(Shadowmap, CompareSampler, Resolution, ShadowCoord + float3(offset.xy, 0.0), int2(0, 0), Slice);
#endif
        if (_nonlinear_depth_test(dist, ShadowCoord.z))
        {
            shadow += dist; 
        }
    }

    return saturate(shadow / (float) g_Gaussian9x9_KernelSize);
}


float shadow_linear_sample_PCF_1x1(Texture2DArray Shadowmap, SamplerState Sampler, in float2 Resolution, in float2 ClipNearFar, in float3 ShadowCoord, in uint Slice, in float Bias)
{
    if (_inside_uv(ShadowCoord.xyz))
    {
        float2 stc = ShadowCoord.xy * Resolution + 0.5.xx;
        float2 tcs = floor(stc);
        ShadowCoord.xy = tcs / Resolution;
        
        float kernelRadius = 1.0;
        float2 texelSize = rcp(Resolution);
        ShadowCoord.xy = clamp(ShadowCoord.xy, texelSize * kernelRadius, 1 - texelSize * kernelRadius);

        float4 distQuad = Shadowmap.GatherRed(Sampler, float3(ShadowCoord.xy, (float) Slice));
        float2 fracShadow = stc - tcs;
        float dist = lerp(lerp(distQuad.w, distQuad.z, fracShadow.x), lerp(distQuad.x, distQuad.y, fracShadow.x), fracShadow.y);
        
        float linearDist = _linearize_depth(dist, ClipNearFar.x, ClipNearFar.y);
        float linearDepth = _linearize_depth(ShadowCoord.z, ClipNearFar.x, ClipNearFar.y);
        if (linearDist + Bias < linearDepth)
        {
            return 1.0;
        }
    }
    
    return 0.0;
}

float shadow_linear_sample_PCF_3x3(Texture2DArray Shadowmap, SamplerState Sampler, in float2 Resolution, in float2 ClipNearFar, in float3 ShadowCoord, in uint Slice, in float2 ScaleFactor, in float Bias)
{
    if (_inside_uv(ShadowCoord.xyz))
    {
        const float kernelRadius = 3.0;
        float2 texelSize = rcp(Resolution);
        ShadowCoord.xy = clamp(ShadowCoord.xy, texelSize * kernelRadius, 1.0 - texelSize * kernelRadius);
        
        float sum = 0;
        [unroll(g_Gaussian3x3_KernelSize)]
        for (uint i = 0; i < g_Gaussian3x3_KernelSize; ++i)
        {
            float2 offset = g_Gaussian3x3_Kernel[i] * texelSize * ScaleFactor;
            float dist = Shadowmap.SampleLevel(Sampler, float3(ShadowCoord.xy + offset, (float) Slice), 0).r;

            float linearDist = _linearize_depth(dist, ClipNearFar.x, ClipNearFar.y);
            float linearDepth = _linearize_depth(ShadowCoord.z, ClipNearFar.x, ClipNearFar.y);
            if (linearDist + Bias < linearDepth)
            {
                sum += 1.0 * g_Gaussian3x3_Weights[i];
            }
        }
        
        return saturate(sum / g_Gaussian3x3_WeightsSum);
    }
    
    return 0.0;
}

float shadow_linear_sample_PCF_5x5(Texture2DArray Shadowmap, SamplerState Sampler, in float2 Resolution, in float2 ClipNearFar, in float3 ShadowCoord, in uint Slice, in float2 ScaleFactor, in float Bias)
{
    if (_inside_uv(ShadowCoord.xyz))
    {
        const float kernelRadius = 5.0;
        float2 texelSize = rcp(Resolution);
        ShadowCoord.xy = clamp(ShadowCoord.xy, texelSize * kernelRadius, 1.0 - texelSize * kernelRadius);
        
        float sum = 0;
        [unroll(g_Gaussian5x5_KernelSize)]
        for (uint i = 0; i < g_Gaussian5x5_KernelSize; ++i)
        {
            float2 offset = g_Gaussian5x5_Kernel[i] * texelSize * ScaleFactor;
            float dist = Shadowmap.SampleLevel(Sampler, float3(ShadowCoord.xy + offset, (float) Slice), 0).r;

            float linearDist = _linearize_depth(dist, ClipNearFar.x, ClipNearFar.y);
            float linearDepth = _linearize_depth(ShadowCoord.z, ClipNearFar.x, ClipNearFar.y);
            if (linearDist + Bias < linearDepth)
            {
                sum += 1.0 * g_Gaussian5x5_Weights[i];
            }
        }
        
        return saturate(sum / g_Gaussian5x5_WeightsSum);
    }
    
    return 0.0;
}

float shadow_linear_sample_PCF_9x9(Texture2DArray Shadowmap, SamplerState Sampler, in float2 Resolution, in float2 ClipNearFar, in float3 ShadowCoord, in uint Slice, in float2 ScaleFactor, in float Bias)
{
    if (_inside_uv(ShadowCoord.xyz))
    {
        const float kernelRadius = 9.0;
        float2 texelSize = rcp(Resolution);
        ShadowCoord.xy = clamp(ShadowCoord.xy, texelSize * kernelRadius, 1.0 - texelSize * kernelRadius);
        
        float sum = 0;
        [unroll(g_Gaussian9x9_KernelSize)]
        for (uint i = 0; i < g_Gaussian9x9_KernelSize; ++i)
        {
            float2 offset = g_Gaussian9x9_Kernel[i] * texelSize * ScaleFactor;
            float dist = Shadowmap.SampleLevel(Sampler, float3(ShadowCoord.xy + offset, (float) Slice), 0).r;

            float linearDist = _linearize_depth(dist, ClipNearFar.x, ClipNearFar.y);
            float linearDepth = _linearize_depth(ShadowCoord.z, ClipNearFar.x, ClipNearFar.y);
            if (linearDist + Bias < linearDepth)
            {
                sum += 1.0 * g_Gaussian9x9_Weights[i];
            }
        }
        
        return saturate(sum / g_Gaussian9x9_WeightsSum);
    }
    
    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_SHADOW_COMMON_HLSL_