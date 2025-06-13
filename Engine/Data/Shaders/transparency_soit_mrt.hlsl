#include "global.hlsli"
#include "gbuffer_common.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> CB_Model : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D textureBaseColor : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D texture0 : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D texture1 : register(t2, space1);
[[vk::binding(6, 1)]] Texture2D texture2 : register(t3, space1);
[[vk::binding(7, 1)]] Texture2D texture3 : register(t4, space1);

///////////////////////////////////////////////////////////////////////////////////////

float dither2x2(int2 pixel)
{
    float4 pattern = float4(0.25, 0.75, 0.75, 0.25);
    return pattern[pixel.x % 2 + (pixel.y % 2) * 2];
}

float hash2d(float2 seed)
{
    return frac(sin(dot(seed, float2(12.9898, 78.233))) * 43758.5453);
}

float hash4d(float4 seed)
{
    return hash2d(float2(hash2d(float2(hash2d(seed.xy), seed.z)), seed.w));
}

// Modified version of Inigo Quilez' function for generating per-fragment alpha-to-coverage sample
// masks, https://twitter.com/iquilezles/status/947717661496041472. Here we use a simple 4D hash to
// randomize the sample mask and rely on TAA to suppress the noise.
int a2c(float alpha, int2 pixel, float3 view_position, int frame_id, bool frontFace)
{
    const int msaa = 4;

    // Dither the input alpha value with a 2x2 checker pattern to increase the number of alpha
    // levels from 5 to 9
    float dither = dither2x2(pixel + int2(int(frontFace), frame_id % 2));
    alpha = clamp(max(0.125, alpha) + (dither - 0.5) / float(msaa), 0.0, 1.0);

    // Compute alpha-to-coverage sample mask
    int mask = ~(0xff << int(alpha * float(msaa) + 0.5)) & 0xf;

    // Randomize the mask
    float rnd = hash4d(float4(view_position, frame_id % 1000));
    int shift = int(msaa * rnd) % msaa;
    mask = (((mask << 4) | mask) >> shift) & 0xf; // barrel shift

    return mask;
}

///////////////////////////////////////////////////////////////////////////////////////

struct PS_MSOIT_STRUCT
{
    [[vk::location(0)]] float4 sample0 : SV_TARGET0;
    [[vk::location(1)]] float4 sample1 : SV_TARGET1;
    [[vk::location(2)]] float4 sample2 : SV_TARGET2;
    [[vk::location(3)]] float4 sample3 : SV_TARGET3;
};

PS_MSOIT_STRUCT msoit_mrt_ps(PS_GBUFFER_STANDARD_INPUT input, bool frontFace : SV_ISFRONTFACE, uint PrimitiveID : SV_PRIMITIVEID)
{
    float3 albedo = textureBaseColor.Sample(samplerState, input.UV).rgb;
    float3 normal = texture0.Sample(samplerState, input.UV).rgb * 2.0 - 1.0;
    float metalness = texture1.Sample(samplerState, input.UV).r;
    float roughness = texture1.Sample(samplerState, input.UV).g;
    
    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Bitangent);
    float3 B = normalize(input.Tangent);
    float3x3 TBN = float3x3(T, B, N);
    normal = mul(TBN, normalize(normal));
    
    float4 color = float4( /*albedo **/CB_Model.tint.rgb * CB_Model.tint.a, CB_Model.tint.a);
    
    //temp
    float2 positionScreenUV = input.Position.xy * (1.0 / CB_Viewport.viewportSize.xy);
    float2 offset = CB_Viewport.random.xy * (PrimitiveID + 1);
    float noise = texture2.Sample(samplerState, positionScreenUV + offset).a;
    if (noise >= color.a)
    {
        discard;
    }
    
    int mask = a2c(color.a, int2(input.Position.xy), input.WorldPos, int(CB_Viewport.time), frontFace);
    
    PS_MSOIT_STRUCT Output;
    Output.sample0 = float4(0.0, 0.0, 0.0, 0.0);
    Output.sample1 = float4(0.0, 0.0, 0.0, 0.0);
    Output.sample2 = float4(0.0, 0.0, 0.0, 0.0);
    Output.sample3 = float4(0.0, 0.0, 0.0, 0.0);
    
    if (mask == 0)
    {
        discard;
    }
    
    if (mask & 0x1)
    {
        Output.sample0 = float4(color.rgb, 1.0);
    }
    if (mask & 0x2)
    {
        Output.sample1 = float4(color.rgb, 1.0);
    }
    
    if (mask & 0x4)
    {
        Output.sample2 = float4(color.rgb, 1.0);
    }
    
    if (mask & 0x8)
    {
        Output.sample3 = float4(color.rgb, 1.0);
    }
    
    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 msoit_resolve_ps(PS_OFFSCREEN_INPUT input) : SV_TARGET0
{
    float4 outputColor = float4(0.0);
    int numSamples = 4;
    float u_weight = 1.0;
    {
        float4 sampleColor = texture0.SampleLevel(samplerState, input.UV, 0);
        sampleColor = (sampleColor.a > 0.0) ? float4(sampleColor.rgb, 1.0) : float4(0.0);
        outputColor += sampleColor;
    }
    {
        float4 sampleColor = texture1.SampleLevel(samplerState, input.UV, 0);
        sampleColor = (sampleColor.a > 0.0) ? float4(sampleColor.rgb, 1.0) : float4(0.0);
        outputColor += sampleColor;
    }
    {
        float4 sampleColor = texture2.SampleLevel(samplerState, input.UV, 0);
        sampleColor = (sampleColor.a > 0.0) ? float4(sampleColor.rgb, 1.0) : float4(0.0);
        outputColor += sampleColor;
    }
    {
        float4 sampleColor = texture3.SampleLevel(samplerState, input.UV, 0);
        sampleColor = (sampleColor.a > 0.0) ? float4(sampleColor.rgb, 1.0) : float4(0.0);
        outputColor += sampleColor;
    }
    
    outputColor.rgb /= max(1.0, outputColor.a);
    outputColor.a /= float(numSamples);
    outputColor.rgb *= outputColor.a;
    
    //float3 baseColor = textureBaseColor.Sample(samplerState, input.UV).rgb;
    float alpha = outputColor.a; // 1.0
    
    float4 finalColor = float4( /* baseColor * (1.0 - outputColor.a) +*/float3(u_weight * outputColor.rgb), alpha);
    return finalColor;
}

///////////////////////////////////////////////////////////////////////////////////////