#ifndef _GLOBAL_HLSL_
#define _GLOBAL_HLSL_

///////////////////////////////////////////////////////////////////////////////////////

#define HLSL 1
#define GLSL 0

#define REVERSED_DEPTH 1

///////////////////////////////////////////////////////////////////////////////////////

float linearize_depth(in float d, in float zNear, in float zFar)
{
    return zNear * zFar / (zNear + d * (zFar - zNear));
}

float3 reconstruct_worldPos(float4x4 invViewiProjection, float2 uv, float depth)
{
    float4 clipPos;
    clipPos.xy = uv * 2.0f - 1.0f;
    clipPos.z = depth;
    clipPos.w = 1.0f;

    float4 viewPos = mul(invViewiProjection, clipPos);
    viewPos /= viewPos.w;
    return viewPos.xyz;
}

float3 srgb_linear(float3 srgb)
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

struct ModelBuffer
{
    float4x4 modelMatrix;
    float4x4 prevModelMatrix;
    float4x4 normalMatrix;
    float4 tint;
    uint64_t objectID;
};

///////////////////////////////////////////////////////////////////////////////////////

#endif //_GLOBAL_HLSL_