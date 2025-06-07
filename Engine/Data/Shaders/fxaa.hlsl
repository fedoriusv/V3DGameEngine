#include "global.hlsl"

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_GREEN_AS_LUMA 1
#define FXAA_QUALITY__PRESET 12
#include "Fxaa3_11.h"

struct PS_INPUT
{
    [[vk::location(0)]] float2 UV : TEXTURE;
};

[[vk::binding(1, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(2, 1)]] Texture2D textureColor   : register(t0, space1);

float4 fxaa_ps(PS_INPUT Input) : SV_TARGET0
{
    uint dx, dy;
    textureColor.GetDimensions(dx, dy);
    float2 rcpro = rcp(float2(dx, dy));

    FxaaTex t;
    t.smpl = samplerState;
    t.tex = textureColor;

    return FxaaPixelShader(Input.UV, 0, t, t, t, rcpro, 0, 0, 0, 1.0, 0.166, 0.0312, 0, 0, 0, 0);

}