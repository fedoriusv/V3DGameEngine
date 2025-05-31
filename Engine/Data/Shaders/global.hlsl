#ifndef _GLOBAL_HLSL_
#define _GLOBAL_HLSL_

struct Viewport
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
    float4   cameraPosition;
    float4   viewportSize;
    float2   cursorPosition;
    uint64_t time;
};

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> viewport : register(b0, space0);

#endif //_GLOBAL_HLSL_