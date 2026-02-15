#ifndef _GLOBAL_HLSL_
#define _GLOBAL_HLSL_

///////////////////////////////////////////////////////////////////////////////////////

#define HLSL 1
#define DX_FORMAT 1

#define GLSL 0
#define GL_FORMAT 0

#ifndef REVERSED_DEPTH
#define REVERSED_DEPTH 1
#endif

#ifndef SHADER_DEBUG
#define SHADER_DEBUG 0
#endif

///////////////////////////////////////////////////////////////////////////////////////

#define CONSTANT_BUFFER(Struct, Name, Set, Binding) \
    [[vk::binding(##Binding, ##Set)]] ConstantBuffer<##Struct> Name : register(b##Binding, space##Set)

#define TEXTURE2D(Name, Set, Binding) \
        [[vk::binding(##Binding, ##Set)]] Texture2D Name : register(t##Binding, space##Set)

#define TEXTURE3D(Name, Set, Binding) \
        [[vk::binding(##Binding, ##Set)]] Texture3D Name : register(t##Binding, space##Set)

#define SAMPLER_STATE(Name, Set, Binding) \
        [[vk::binding(##Binding, ##Set)]] SamplerState Name : register(t##Binding, space##Set)

///////////////////////////////////////////////////////////////////////////////////////

static const float PI = 3.14159265;

///////////////////////////////////////////////////////////////////////////////////////

struct VS_SIMPLE_INPUT
{
    [[vk::location(0)]] float3 Position : IN_POSITION;
    [[vk::location(1)]] float3 Normal   : IN_NORMAL;
    [[vk::location(2)]] float2 UV       : IN_TEXTURE;
};

struct VS_SIMPLE_OUTPUT
{
    float4                     Position : SV_POSITION;
    [[vk::location(0)]] float4 ClipPos  : CLIPPOS;
    [[vk::location(2)]] float3 WorldPos : WORLDPOS;
    [[vk::location(3)]] float3 Normal   : NORMAL;
    [[vk::location(6)]] float2 UV       : TEXTURE;
};

typedef VS_SIMPLE_OUTPUT PS_SIMPLE_INPUT;

///////////////////////////////////////////////////////////////////////////////////////

float linearize_depth(in float depth, in float nearPlane, in float farPlane)
{
#if REVERSED_DEPTH
    return (nearPlane * farPlane) / (nearPlane + depth * (farPlane - nearPlane));
#else
    return (nearPlane * farPlane) / (farPlane - depth * (farPlane - nearPlane));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

float3 srgb_linear(in float3 srgb)
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

uint cubemap_face_id(in float3 Dir)
{
    float3 a = abs(Dir);
    if (a.x > a.y && a.x > a.z)
    {
        return Dir.x > 0 ? 0 : 1; // +X -X
    }
    else if (a.y > a.z)
    {
        return Dir.y > 0 ? 2 : 3; // +Y -Y
    }
    else
    {
        return Dir.z > 0 ? 4 : 5; // +Z -Z
    }
}

float2 cubemap_face_UV(float3 Dir, uint Face)
{
    float2 uv;
    float ma;
    switch (Face)
    {
        case 0: // +X
            ma = abs(Dir.x);
            uv = float2(-Dir.z, -Dir.y) / ma;
            break;

        case 1: // -X
            ma = abs(Dir.x);
            uv = float2(Dir.z, -Dir.y) / ma;
            break;

        case 2: // +Y
            ma = abs(Dir.y);
            uv = float2(Dir.x, Dir.z) / ma;
            break;

        case 3: // -Y
            ma = abs(Dir.y);
            uv = float2(Dir.x, -Dir.z) / ma;
            break;

        case 4: // +Z
            ma = abs(Dir.z);
            uv = float2(Dir.x, -Dir.y) / ma;
            break;

        default: // -Z
            ma = abs(Dir.z);
            uv = float2(-Dir.x, -Dir.y) / ma;
            break;
    }

    return uv * 0.5 + 0.5;
}

///////////////////////////////////////////////////////////////////////////////////////

struct ModelBuffer
{
    float4x4 modelMatrix;
    float4x4 prevModelMatrix;
    float4x4 normalMatrix;
    float4   tintColour;
    uint64_t objectID;
};

///////////////////////////////////////////////////////////////////////////////////////

#endif //_GLOBAL_HLSL_