#ifndef _GLOBAL_HLSL_
#define _GLOBAL_HLSL_

struct Viewport
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
    float4x4 prevProjectionMatrix;
    float4x4 prevViewMatrix;
    float4   cameraPosition;
    float2   viewportSize;
    float2   cursorPosition;
    float4   random;
    uint64_t time;
    float2   _unused;
};

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> viewport : register(b0, space0);

#endif //_GLOBAL_HLSL_