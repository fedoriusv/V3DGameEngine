#ifndef _VIEWPORT_HLSL_
#define _VIEWPORT_HLSL_

#include "global.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

struct Viewport
{
    float4x4 projectionMatrix;
    float4x4 invProjectionMatrix;
    float4x4 viewMatrix;
    float4x4 invViewMatrix;
    float4x4 prevProjectionMatrix;
    float4x4 prevViewMatrix;
    float2   cameraJitter;
    float2   prevCameraJitter;
    float4   cameraPosition;
    float4   random;
    float2   viewportSize;
    float2   clipNearFar;
    float2   cursorPosition;
    uint64_t time;
};

///////////////////////////////////////////////////////////////////////////////////////

float3 reconstruct_WorldPos(float4x4 invProjection, float4x4 invView, float2 uv, float depth)
{
    float4 clipPos;
    clipPos.xy = uv * 2.0f - 1.0f;
    clipPos.z = depth;
    clipPos.w = 1.0f;

    float4 viewSpacePos = mul(invProjection, clipPos);
    viewSpacePos /= viewSpacePos.w;
    
    float4 worldSpacePos = mul(invView, viewSpacePos);
    return worldSpacePos.xyz;
}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_VIEWPORT_HLSL_