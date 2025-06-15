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
    float2   clipNearFar;
    float4   random;
    float2   cursorPosition;
    uint64_t time;
};

float linearize_depth(in float d, in float zNear, in float zFar)
{
    return zNear * zFar / (zNear + d * (zFar - zNear));
}

#endif //_GLOBAL_HLSL_