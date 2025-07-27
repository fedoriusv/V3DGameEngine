#ifndef _GLOBAL_HLSL_
#define _GLOBAL_HLSL_

///////////////////////////////////////////////////////////////////////////////////////

#define HLSL 1
#define GLSL 0

#define REVERSED_DEPTH 1

///////////////////////////////////////////////////////////////////////////////////////

struct VS_SIMPLE_INPUT
{
    [[vk::location(0)]] float3 Position : IN_POSITION;
    [[vk::location(1)]] float3 Normal   : IN_NORMAL;
    [[vk::location(2)]] float2 UV       : IN_TEXTURE;
};

struct VS_SIMPLE_OUTPUT
{
    float4                     Position  : SV_POSITION;
    [[vk::location(0)]] float4 ClipPos   : CLIPPOS;
    [[vk::location(2)]] float3 WorldPos  : WORLDPOS;
    [[vk::location(3)]] float3 Normal    : NORMAL;
    [[vk::location(6)]] float2 UV        : TEXTURE;
};

typedef VS_SIMPLE_OUTPUT PS_SIMPLE_INPUT;

///////////////////////////////////////////////////////////////////////////////////////

float linearize_depth(in float d, in float zNear, in float zFar)
{
    return zNear * zFar / (zNear + d * (zFar - zNear));
}

///////////////////////////////////////////////////////////////////////////////////////

float3 srgb_linear(float3 srgb)
{
    float3 linearLow = srgb / 12.92;
    float3 linearHigh = pow((srgb + 0.055) / 1.055, 2.4);
    return lerp(linearLow, linearHigh, step(0.04045, srgb));
}

///////////////////////////////////////////////////////////////////////////////////////

float4 srgb_linear(float4 srgb)
{
    return float4(srgb_linear(srgb.rgb), srgb.a);
}

///////////////////////////////////////////////////////////////////////////////////////

float3 srgb_linear_approx(float3 srgb)
{
    return pow(srgb, 2.2); // or use 2.0 for faster, rougher approximation
}

///////////////////////////////////////////////////////////////////////////////////////

float3 linear_srgb(float3 lin)
{
    float3 srgbLow = lin * 12.92;
    float3 srgbHigh = 1.055 * pow(lin, 1.0 / 2.4) - 0.055;
    return lerp(srgbLow, srgbHigh, step(0.0031308, lin));
}

///////////////////////////////////////////////////////////////////////////////////////

float4 linear_srgb(float4 lin)
{
    return float4(linear_srgb(lin.rgb), lin.a);
}

///////////////////////////////////////////////////////////////////////////////////////

struct ModelBuffer
{
    float4x4 modelMatrix;
    float4x4 prevModelMatrix;
    float4x4 normalMatrix;
    float4   tint;
    uint64_t objectID;
};

///////////////////////////////////////////////////////////////////////////////////////

#endif //_GLOBAL_HLSL_