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

bool is_inside_uv(in float3 Coord)
{
    return all(Coord >= 0.0.xxx) && all(Coord <= 1.0.xxx);
}

bool is_outside_uv(in float3 Coord)
{
    return any(Coord < 0.0.xxx) || any(Coord > 1.0.xxx);
}
///////////////////////////////////////////////////////////////////////////////////////

float linearize_depth(in float Depth, in float NearPlane, in float FarPlane)
{
#if REVERSED_DEPTH
    return (NearPlane * FarPlane) / (NearPlane + Depth * (FarPlane - NearPlane));
#else
    return (NearPlane * FarPlane) / (FarPlane - Depth * (FarPlane - NearPlane));
#endif
}

bool depth_test_nonlinear(in float Depth, in float TestValue)
{
#if REVERSED_DEPTH
    return Depth > TestValue; // [1..0]
#else
    return Depth < TestValue; // [0..1]
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

float3 srgb_linear(in float3 srgb)
{
    float3 linearLow = srgb / 12.92;
    float3 linearHigh = pow((srgb + 0.055) / 1.055, 2.4);
    return lerp(linearLow, linearHigh, step(0.04045, srgb));
}

float4 srgb_linear(float4 srgb)
{
    return float4(srgb_linear(srgb.rgb), srgb.a);
}

float3 srgb_linear_approx(float3 srgb)
{
    return pow(srgb, 2.2); // or use 2.0 for faster, rougher approximation
}

float3 linear_srgb(float3 lin)
{
    float3 srgbLow = lin * 12.92;
    float3 srgbHigh = 1.055 * pow(lin, 1.0 / 2.4) - 0.055;
    return lerp(srgbLow, srgbHigh, step(0.0031308, lin));
}

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

// Gaussian3x3
static const int g_Gaussian3x3_KernelSize = 9;
static const float2 g_Gaussian3x3_Kernel[g_Gaussian3x3_KernelSize] =
    {
        float2(-1.0, -1.0), float2(0.0, -1.0), float2(1.0, -1.0),
        float2(-1.0,  0.0), float2(0.0,  0.0), float2(1.0,  0.0),
        float2(-1.0,  1.0), float2(0.0,  1.0), float2(1.0,  1.0)
    };

static const int g_Gaussian3x3_WeightsSum = 16;
static const int g_Gaussian3x3_Weights[g_Gaussian3x3_KernelSize] =
    {
        1, 2, 1,
        2, 4, 2,
        1, 2, 1
    };

// Gaussian5x5
static const int g_Gaussian5x5_KernelSize = 25;
static const float2 g_Gaussian5x5_Kernel[g_Gaussian5x5_KernelSize] =
    {
        float2(-2.0, -2.0), float2(-1.0, -2.0), float2(0.0, -2.0), float2(1.0, -2.0), float2(2.0, -2.0),
        float2(-2.0, -1.0), float2(-1.0, -1.0), float2(0.0, -1.0), float2(1.0, -1.0), float2(2.0, -1.0),
        float2(-2.0,  0.0), float2(-1.0,  0.0), float2(0.0,  0.0), float2(1.0,  0.0), float2(2.0,  0.0),
        float2(-2.0,  1.0), float2(-1.0,  1.0), float2(0.0,  1.0), float2(1.0,  1.0), float2(2.0,  1.0),
        float2(-2.0,  2.0), float2(-1.0,  2.0), float2(0.0,  2.0), float2(1.0,  2.0), float2(2.0,  2.0),
    };

static const int g_Gaussian5x5_WeightsSum = 256;
static const int g_Gaussian5x5_Weights[g_Gaussian5x5_KernelSize] =
    {
        1, 4,  6,  4,  1,
        4, 16, 24, 16, 4,
        6, 24, 36, 24, 6,
        4, 16, 24, 16, 4,
        1, 4,  6,  4,  1
    };

// Gaussian9x9
static const int g_Gaussian9x9_KernelSize = 81;
static const float2 g_Gaussian9x9_Kernel[g_Gaussian9x9_KernelSize] =
    {
        float2(-4.0, -4.0), float2(-3.0, -4.0), float2(-2.0, -4.0), float2(-1.0, -4.0), float2(0.0, -4.0), float2(1.0, -4.0), float2(2.0, -4.0), float2(3.0, -4.0), float2(4.0, -4.0),
        float2(-4.0, -3.0), float2(-3.0, -3.0), float2(-2.0, -3.0), float2(-1.0, -3.0), float2(0.0, -3.0), float2(1.0, -3.0), float2(2.0, -3.0), float2(3.0, -3.0), float2(4.0, -3.0),
        float2(-4.0, -2.0), float2(-3.0, -2.0), float2(-2.0, -2.0), float2(-1.0, -2.0), float2(0.0, -2.0), float2(1.0, -2.0), float2(2.0, -2.0), float2(3.0, -2.0), float2(4.0, -2.0),
        float2(-4.0, -1.0), float2(-3.0, -1.0), float2(-2.0, -1.0), float2(-1.0, -1.0), float2(0.0, -1.0), float2(1.0, -1.0), float2(2.0, -1.0), float2(3.0, -1.0), float2(4.0, -1.0),
        float2(-4.0,  0.0), float2(-3.0,  0.0), float2(-2.0,  0.0), float2(-1.0,  0.0), float2(0.0,  0.0), float2(1.0,  0.0), float2(2.0,  0.0), float2(3.0,  0.0), float2(4.0,  0.0),
        float2(-4.0,  1.0), float2(-3.0,  1.0), float2(-2.0,  1.0), float2(-1.0,  1.0), float2(0.0,  1.0), float2(1.0,  1.0), float2(2.0,  1.0), float2(3.0,  1.0), float2(4.0,  1.0),
        float2(-4.0,  2.0), float2(-3.0,  2.0), float2(-2.0,  2.0), float2(-1.0,  2.0), float2(0.0,  2.0), float2(1.0,  2.0), float2(2.0,  2.0), float2(3.0,  2.0), float2(4.0,  2.0),
        float2(-4.0,  3.0), float2(-3.0,  3.0), float2(-2.0,  3.0), float2(-1.0,  3.0), float2(0.0,  3.0), float2(1.0,  3.0), float2(2.0,  3.0), float2(3.0,  3.0), float2(4.0,  3.0),
        float2(-4.0,  4.0), float2(-3.0,  4.0), float2(-2.0,  4.0), float2(-1.0,  4.0), float2(0.0,  4.0), float2(1.0,  4.0), float2(2.0,  4.0), float2(3.0,  4.0), float2(4.0,  4.0)
    };

static const int g_Gaussian9x9_WeightsSum = 65536;
static const int g_Gaussian9x9_Weights[g_Gaussian9x9_KernelSize] =
    {
        1,  8,   28,   56,   70,   56,   28,   8,   1,
        8,  64,  224,  448,  560,  448,  224,  64,  8,
        28, 224, 784,  1568, 1960, 1568, 784,  224, 28,
        56, 448, 1568, 3136, 3920, 3136, 1568, 448, 56,
        70, 560, 1960, 3920, 4900, 3920, 1960, 560, 70,
        56, 448, 1568, 3136, 3920, 3136, 1568, 448, 56,
        28, 224, 784,  1568, 1960, 1568, 784,  224, 28,
        8,  64,  224,  448,  560,  448,  224,  64,  8,
        1,  8,   28,   56,   70,   56,   28,   8,   1
    };

///////////////////////////////////////////////////////////////////////////////////////

struct ModelBuffer
{
    float4x4 modelMatrix;
    float4x4 prevModelMatrix;
    float4x4 normalMatrix;
    float4   tintColour;
    //float    bumpHeight;
    uint64_t objectID;
};

///////////////////////////////////////////////////////////////////////////////////////

#endif //_GLOBAL_HLSL_