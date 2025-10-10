#include "global.hlsli"
#include "viewport.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport : register(b0, space0);

[[vk::binding(2, 1)]] SamplerState s_SamplerState          : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D t_SkyboxTexture            : register(t0, space1);

///////////////////////////////////////////////////////////////////////////////////////

VS_SIMPLE_OUTPUT skybox_vs(VS_SIMPLE_INPUT Input)
{
    VS_SIMPLE_OUTPUT Output;
    
    float4x4 skypos =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    skypos[3].xyz = cb_Viewport.cameraPosition.xyz;
    
    float4 position = mul(transpose(skypos), float4(Input.Position, 1.0f));
    
    Output.ClipPos = mul(cb_Viewport.projectionMatrix, mul(cb_Viewport.viewMatrix, position));
    Output.WorldPos = Input.Position;
    Output.Normal = Input.Normal;
    Output.UV = -Input.UV; 

    Output.Position = Output.ClipPos;

    return Output;
}

float4 skybox_ps(PS_SIMPLE_INPUT Input) : SV_TARGET0
{
    float3 color = t_SkyboxTexture.Sample(s_SamplerState, Input.UV).xyz;
    return float4(color, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////