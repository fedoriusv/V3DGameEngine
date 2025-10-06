#include "global.hlsli"
#include "viewport.hlsli"
#include "gbuffer_common.hlsli"
#include "lighting_common.hlsli"
#include "offscreen_common.hlsli"
#include "vfx_common.hlsli"

//Enable SEPARATE_MATERIALS define to use Metalness and Roughness in separate textures
//#define SEPARATE_MATERIALS 1

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> cb_Model : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState s_SamplerState          : register(s0, space1);
#if SEPARATE_MATERIALS
[[vk::binding(3, 1)]] Texture2D t_TextureAlbedo            : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D t_TextureNormal            : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D t_TextureRoughness         : register(t2, space1);
[[vk::binding(6, 1)]] Texture2D t_TextureMetalness         : register(t3, space1);
[[vk::binding(7, 1)]] Texture2D t_TextureMask              : register(t4, space1);
#else
[[vk::binding(3, 1)]] Texture2D t_TextureAlbedo            : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D t_TextureNormal            : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D t_TextureMaterial          : register(t2, space1);
[[vk::binding(7, 1)]] Texture2D t_TextureMask              : register(t3, space1);
#endif

///////////////////////////////////////////////////////////////////////////////////////

VS_GBUFFER_STANDARD_OUTPUT gbuffer_standard_vs(VS_GBUFFER_STANDARD_INPUT Input)
{
    return _gbuffer_standard_vs(Input, cb_Viewport, cb_Model);
}

///////////////////////////////////////////////////////////////////////////////////////

VS_GBUFFER_STANDARD_OUTPUT gbuffer_billboard_vs(uint VertexID : SV_VERTEXID)
{
    VS_SIMPLE_OUTPUT SimpleOutput = _billboard_vs(cb_Viewport, cb_Model, VertexID);

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
#if SEPARATE_MATERIALS
    float3 albedo = srgb_linear(t_TextureAlbedo.Sample(s_SamplerState, Input.UV).rgb);
    float3 normal = t_TextureNormal.Sample(s_SamplerState, Input.UV).rgb * 2.0 - 1.0;
    float roughness = t_TextureRoughness.Sample(s_SamplerState, Input.UV).r;
    float metalness = t_TextureMetalness.Sample(s_SamplerState, Input.UV).r;
#else
    float3 albedo = srgb_linear(t_TextureAlbedo.Sample(s_SamplerState, Input.UV).rgb);
    float3 normal = t_TextureNormal.Sample(s_SamplerState, Input.UV).rgb * 2.0 - 1.0;
    float3 materials = t_TextureMaterial.Sample(s_SamplerState, Input.UV).rgb;
    float roughness = materials.r;
    float metalness = materials.g;
#endif
    return _gbuffer_standard_ps(Input, cb_Viewport, cb_Model, albedo, normal, roughness, metalness);
}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT gbuffer_masked_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
#if SEPARATE_MATERIALS
    float4 baseColor = t_TextureAlbedo.Sample(s_SamplerState, Input.UV);
    float3 albedo = srgb_linear(baseColor.rgb);
    float3 normal = t_TextureNormal.Sample(s_SamplerState, Input.UV).rgb * 2.0 - 1.0;
    float roughness = t_TextureRoughness.Sample(s_SamplerState, Input.UV).r;
    float metalness = t_TextureMetalness.Sample(s_SamplerState, Input.UV).r;
#else
    float4 baseColor = t_TextureAlbedo.Sample(s_SamplerState, Input.UV);
    float3 albedo = srgb_linear(baseColor.rgb);
    float3 normal = t_TextureNormal.Sample(s_SamplerState, Input.UV).rgb * 2.0 - 1.0;
    float3 materials = t_TextureMaterial.Sample(s_SamplerState, Input.UV).rgb;
    float roughness = materials.r;
    float metalness = materials.g;
#endif
    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(Input, cb_Viewport, cb_Model, albedo, baseColor.a, normal, roughness, metalness);
    
    float3 color = GBubfferStruct.BaseColor.rgb;
    float opacity = GBubfferStruct.BaseColor.a;
    
    const float clipValue = 0.333;
    
    float2 pos = Input.Position.xy + cb_Viewport.cameraJitter * 10000;
    uint value = ((uint) (pos.x) + 2u * (uint) (pos.y)) % 5u;
    float2 screenSpaceUV = Input.Position.xy * (1.0 / cb_Viewport.viewportSize.xy);
    float noise = t_TextureMask.SampleLevel(s_SamplerState, screenSpaceUV, 0).r;
    
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
    float3 color = t_TextureAlbedo.Sample(s_SamplerState, Input.UV).rgb;
    
    PS_GBUFFER_STRUCT Output;
    
    Output.BaseColor = float4(color.rgb * cb_Model.tintColour.rgb, 1.0);
    Output.Normal = float4(0.0, 0.0, 0.0, 0.0);
    Output.Material = float4(0.0, 0.0, (float)cb_Model.objectID, 0.0);
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
    float id = (float)cb_Model.objectID;
    return id;
}

///////////////////////////////////////////////////////////////////////////////////////
