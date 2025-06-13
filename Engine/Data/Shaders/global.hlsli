#ifndef _GLOBAL_HLSL_
#define _GLOBAL_HLSL_

#define HLSL 1
#define GLSL 0

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

#endif //_GLOBAL_HLSL_