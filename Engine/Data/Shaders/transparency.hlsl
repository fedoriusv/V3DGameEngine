#include "global.hlsli"
#include "gbuffer_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(2, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D textureAlbedo : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D textureNormal : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D textureMaterial : register(t2, space1);

typedef VS_GBUFFER_STANDARD_OUTPUT PS_INPUT;

[[vk::location(0)]] float4 transparency_ps(PS_INPUT Input) : SV_TARGET0
{
    float3 albedo = textureAlbedo.Sample(samplerState, Input.UV).rgb;
    float3 normal = textureNormal.Sample(samplerState, Input.UV).rgb * 2.0 - 1.0;
    float metalness = textureMaterial.Sample(samplerState, Input.UV).r;
    float roughness = textureMaterial.Sample(samplerState, Input.UV).g;
    
    float3 N = normalize(Input.Normal);
    float3 T = normalize(Input.Bitangent);
    float3 B = normalize(Input.Tangent);
    float3x3 TBN = float3x3(T, B, N);
    normal = mul(TBN, normalize(normal));
    
    float4 color = float4(albedo * CB_Model.tint.rgb, CB_Model.tint.a);
    return color;
}

///////////////////////////////////////////////////////////////////////////////////////

//[[vk::location(0)]] float4 stochastic_mask_ps(PS_INPUT Input) : SV_TARGET0
//{
//    float3 albedo = textureAlbedo.Sample(samplerState, Input.UV).rgb;
//    float3 normal = textureNormal.Sample(samplerState, Input.UV).rgb * 2.0 - 1.0;
//    float metalness = textureMaterial.Sample(samplerState, Input.UV).r;
//    float roughness = textureMaterial.Sample(samplerState, Input.UV).g;
    
//    float3 N = normalize(Input.Normal);
//    float3 T = normalize(Input.Bitangent);
//    float3 B = normalize(Input.Tangent);
//    float3x3 TBN = float3x3(T, B, N);
//    normal = mul(TBN, normalize(normal));
    
//    float alpha = CB_Model.tint.a;
    
//    float rand = blueNoiseTexture.Sample(samplerState, Input.UV).r;
//    float mask = (rand < alpha) ? 1.0f : 0.0f;
    
//    float4 color = float4(albedo * CB_Model.tint.rgb * mask, mask);
//    return color;
//}

///////////////////////////////////////////////////////////////////////////////////////

//[[vk::location(0)]] float4 stochastic_test_ps(PS_INPUT Input) : SV_TARGET0
//{
//    float3 albedo = textureAlbedo.Sample(samplerState, Input.UV).rgb;
//    float3 normal = textureNormal.Sample(samplerState, Input.UV).rgb * 2.0 - 1.0;
//    float metalness = textureMaterial.Sample(samplerState, Input.UV).r;
//    float roughness = textureMaterial.Sample(samplerState, Input.UV).g;
    
//    float3 N = normalize(Input.Normal);
//    float3 T = normalize(Input.Bitangent);
//    float3 B = normalize(Input.Tangent);
//    float3x3 TBN = float3x3(T, B, N);
//    normal = mul(TBN, normalize(normal));
    
//    float2 screenUV = Input.Position.xy / viewport.viewportSize.zw;
//    float test = alphaTestTexture.Sample(samplerState, screenUV).a;
//    if (test == 0.0)
//    {
//        discard;
//    }

//    float4 color = float4(albedo * CB_Model.tint.rgb, 1.0);
//    return color;
//}

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float transparency_alpha_ps(PS_INPUT Input) : SV_TARGET0
{
    float alpha = CB_Model.tint.a;
    return alpha;
}

///////////////////////////////////////////////////////////////////////////////////////

float rand(float2 seed)
{
    return frac(sin(dot(seed, float2(127.1, 311.7))) * 43758.5453123);
}

[[vk::binding(6, 1)]] Texture2D blueNoiseTexture : register(t3, space1);

void stochastic_depth_ps(PS_INPUT Input, uint PrimitiveID : SV_PRIMITIVEID)
{
    float2 positionScreenUV = Input.Position.xy * (1.0 / viewport.viewportSize.xy);

    //float3 alpha = textureAlbedo.Sample(samplerState, Input.UV).a;
    float alpha = CB_Model.tint.a;

    float2 noiseSize = viewport.viewportSize.xy / 64.0;
    const float S = 8.0;
    float noiseAccum = 0.0;
    for (int s = 0; s < S; ++s)
    {
        float2 offset = viewport.random.xy * (PrimitiveID + 1) * (s + 1);
        //float noise = rand(Input.Position.xy * viewport.random.xy);
        float noise = blueNoiseTexture.Sample(samplerState, positionScreenUV /*Input.UV*/ + offset).a;
        noiseAccum += noise;
        //if (noise >= alpha)
        //{
        //    discard;
        //}
    }
    
    noiseAccum = noiseAccum / S;
    if (noiseAccum >= alpha)
    {
        discard;
    }
}

//uint stochastic_depth_ps(PS_INPUT Input, uint PrimitiveID : SV_PRIMITIVEID) : SV_COVERAGE
//{
//    float2 positionScreenUV = Input.Position.xy * (1.0 / viewport.viewportSize.zw);

//    //float3 alpha = textureAlbedo.Sample(samplerState, Input.UV).a;
//    float alpha = CB_Model.tint.a;
    
//    uint mask = 0;
//    for (int s = 0; s < 8; ++s)
//    {
//        float2 offset = viewport.viewportSize.zw / 64.0 * (PrimitiveID + 1) * (s + 1);
//        //float blueNoise = blueNoiseTexture.Sample(samplerState, positionScreenUV * viewport.random.xy + offset).a;
//        float blueNoise = blueNoiseTexture.Sample(samplerState, positionScreenUV + viewport.random.xy).a;
//        if (alpha > blueNoise)
//        {
//            mask |= (1 << s);
//        }
//    }
    
//    return mask;
//}

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 transparency_accumulate_color_ps(PS_INPUT Input) : SV_TARGET0
{
    float3 albedo = textureAlbedo.Sample(samplerState, Input.UV).rgb;
    float3 normal = textureNormal.Sample(samplerState, Input.UV).rgb * 2.0 - 1.0;
    float metalness = textureMaterial.Sample(samplerState, Input.UV).r;
    float roughness = textureMaterial.Sample(samplerState, Input.UV).g;
    
    float3 N = normalize(Input.Normal);
    float3 T = normalize(Input.Bitangent);
    float3 B = normalize(Input.Tangent);
    float3x3 TBN = float3x3(T, B, N);
    normal = mul(TBN, normalize(normal));
    
    float4 color = float4(albedo * CB_Model.tint.rgb * CB_Model.tint.a, CB_Model.tint.a);
    return color;
}

///////////////////////////////////////////////////////////////////////////////////////

struct PS_OFFSCREEN_INPUT
{
    [[vk::location(0)]] float2 UV : TEXTURE;
};

[[vk::binding(3, 1)]] Texture2D textureBaseColor : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D textureTotalAlpha : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D textureAccumulateColor : register(t2, space1);

[[vk::location(0)]] float4 stochastic_transparency_final_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float totalAlpha = textureTotalAlpha.Sample(samplerState, Input.UV).r;
    float correctedAlpha = 1.0 - exp(-totalAlpha);
    
    float3 opaqueColor = textureBaseColor.Sample(samplerState, Input.UV).rgb;
    float4 transparencyColor = textureAccumulateColor.Sample(samplerState, Input.UV);
    
    float3 color = opaqueColor.rgb * (1 - /*correctedAlpha*/transparencyColor.a) + transparencyColor.rgb;
    //float3 color = lerp(opaqueColor.rgb, transparencyColor.rgb, correctedAlpha);
    
    return float4(color, 1.0);
    
}