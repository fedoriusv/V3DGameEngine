#ifndef _VIEWPORT_HLSL_
#define _VIEWPORT_HLSL_

#include "global.hlsli"

struct Viewport
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
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

#endif //_VIEWPORT_HLSL_