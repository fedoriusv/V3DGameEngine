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

#define VIEWPORT_SET 0

///////////////////////////////////////////////////////////////////////////////////////

float3 _reconstruct_world_pos(in float4x4 InvProjection, in float4x4 InvView, in float2 UV, in float Depth)
{
    float4 NDC;
    NDC.xy = float2(UV.x * 2.0f - 1.0f, -(UV.y * 2.0f - 1.0f));
    NDC.z = Depth;
    NDC.w = 1.0f;

    float4 viewSpacePos = mul(InvProjection, NDC);
    viewSpacePos /= viewSpacePos.w;
    
    float4 worldSpacePos = mul(InvView, viewSpacePos);
    return worldSpacePos.xyz;
}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_VIEWPORT_HLSL_