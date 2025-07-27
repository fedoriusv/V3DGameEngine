#include "global.hlsli"
#include "viewport.hlsli"
#include "gbuffer_common.hlsli"
#include "lighting_common.hlsli"
#include "offscreen_common.hlsli"
#include "vfx_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> CB_Model : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState samplerState            : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D textureAlbedo              : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D textureNormal              : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D textureMetalness           : register(t2, space1);
[[vk::binding(6, 1)]] Texture2D textureRoughness           : register(t3, space1);
[[vk::binding(7, 1)]] Texture2D textureMask                : register(t4, space1);

///////////////////////////////////////////////////////////////////////////////////////

VS_GBUFFER_STANDARD_OUTPUT gbuffer_standard_vs(VS_GBUFFER_STANDARD_INPUT Input)
{
    return _gbuffer_standard_vs(Input, CB_Viewport, CB_Model);
}

///////////////////////////////////////////////////////////////////////////////////////

VS_GBUFFER_STANDARD_OUTPUT gbuffer_billboard_vs(uint VertexID : SV_VERTEXID)
{
    VS_SIMPLE_OUTPUT SimpleOutput = _billboard_vs(CB_Viewport, CB_Model, VertexID);

    VS_GBUFFER_STANDARD_OUTPUT Output;
    
    Output.ClipPos = SimpleOutput.ClipPos;
    Output.PrevClipPos = SimpleOutput.ClipPos;
    Output.WorldPos = SimpleOutput.WorldPos;
    Output.Normal = SimpleOutput.Normal;
    Output.Tangent = float3(0.0, 0.0, 0.0);
    Output.Bitangent = float3(0.0, 0.0, 0.0);
    Output.UV = SimpleOutput.UV;
    
    Output.Position = Output.ClipPos;

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT gbuffer_standard_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    float3 albedo = srgb_linear(textureAlbedo.Sample(samplerState, Input.UV).rgb);
    float3 normal = textureNormal.Sample(samplerState, Input.UV).rgb * 2.0 - 1.0;
    float metalness = textureMetalness.Sample(samplerState, Input.UV).r;
    float roughness = textureRoughness.Sample(samplerState, Input.UV).r;
    
    return _gbuffer_standard_ps(Input, CB_Viewport, CB_Model, albedo, normal, metalness, roughness);
}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT gbuffer_masked_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    float3 albedo = srgb_linear(textureAlbedo.Sample(samplerState, Input.UV).rgb);
    float3 normal = textureNormal.Sample(samplerState, Input.UV).rgb * 2.0 - 1.0;
    float metalness = textureMetalness.Sample(samplerState, Input.UV).r;
    float roughness = textureRoughness.Sample(samplerState, Input.UV).r;
    
    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(Input, CB_Viewport, CB_Model, albedo, normal, metalness, roughness);
    
    float3 color = GBubfferStruct.BaseColor.rgb;
    float opacity = GBubfferStruct.BaseColor.a;
    
    const float clipValue = 0.333;
    
    float2 pos = Input.Position.xy + CB_Viewport.cameraJitter * 10000;
    uint value = ((uint) (pos.x) + 2u * (uint) (pos.y)) % 5u;
    float2 screenSpaceUV = Input.Position.xy * (1.0 / CB_Viewport.viewportSize.xy);
    float noise = textureMask.SampleLevel(samplerState, screenSpaceUV, 0).r;
    
    float mask = (((float) value + noise * 0.0) * 0.16665) + opacity - 0.5;
    if (mask < clipValue)
    {
        discard;
    }
    
    return GBubfferStruct;

}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT gbuffer_unlit_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    float4 color = textureAlbedo.Sample(samplerState, Input.UV);
    
    PS_GBUFFER_STRUCT Output;
    
    Output.BaseColor = color * CB_Model.tint;
    Output.Normal = float4(0.0, 0.0, 0.0, 0.0);
    Output.Material = float4(0.0, 0.0, 0.0, 0.0);
    Output.Velocity = float2(0.0, 0.0);
    
    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

void gbuffer_depth_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    //nothing
}

///////////////////////////////////////////////////////////////////////////////////////

float gbuffer_selection_ps(PS_GBUFFER_STANDARD_INPUT Input) : SV_TARGET0
{
    float id = (float)CB_Model.objectID;
    return id;
}

///////////////////////////////////////////////////////////////////////////////////////
