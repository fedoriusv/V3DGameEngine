#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"

#ifndef FXAA_PC
#   define FXAA_PC 1
#endif

#ifndef FXAA_HLSL_5
#   define FXAA_HLSL_5 1
#endif

#ifndef FXAA_GREEN_AS_LUMA
#   define FXAA_GREEN_AS_LUMA 1
#endif

#ifndef FXAA_QUALITY__PRESET
#   define FXAA_QUALITY__PRESET 12
#endif

#include "Fxaa3_11.h"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport  : register(b0, space0);

[[vk::binding(1, 1)]] SamplerState s_SamplerState           : register(s0, space1);
[[vk::binding(2, 1)]] Texture2D t_TextureColor              : register(t0, space1);

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 fxaa_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float2 rcpro = rcp(cb_Viewport.viewportSize);

    FxaaTex t;
    t.smpl = s_SamplerState;
    t.tex = t_TextureColor;

    return FxaaPixelShader(Input.UV, 0, t, t, t, rcpro, 0, 0, 0, 1.0, 0.166, 0.0312, 0, 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////