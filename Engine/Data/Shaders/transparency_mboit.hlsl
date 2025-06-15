#include "global.hlsli"
#include "gbuffer_common.hlsli"
#include "offscreen_common.hlsli"

#include "MomentMath.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> CB_Model : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D texture0 : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D texture1 : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D texture2 : register(t2, space1);
[[vk::binding(6, 1)]] Texture2D texture3 : register(t3, space1);
[[vk::binding(7, 1)]] Texture2D texture4 : register(t4, space1);

///////////////////////////////////////////////////////////////////////////////////////

static const float MIN_COVERAGE = 0.001;
static const float MAX_COVERAGE = 1.0 - MIN_COVERAGE;
static const float EMISSIVE_ELEMENT_COVERAGE = MIN_COVERAGE + 0.0005;
static const float CLIP_THRESHOLD = 0; //0.00100050033f;

//Convert any depth z to unit range [0..1]. Logarithmic warp helps give equal relative precision at all scales
float linearize_depth_log(in float z, in float zNear, in float zFar)
{
    return log(z / zNear) / log(zFar / zNear);
}

struct PS_MBOIT_STRUCT
{
    [[vk::location(0)]] float OpticalDepth : SV_TARGET0;
    [[vk::location(1)]] float4 Moments_p1234 : SV_TARGET1;
};

PS_MBOIT_STRUCT mboit_pass1_ps(PS_GBUFFER_STANDARD_INPUT input)
{
    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(input, CB_Viewport, CB_Model, texture0, texture1, texture2, samplerState);
    
    float3 color = GBubfferStruct.BaseColor.rgb;
    float opacity = GBubfferStruct.BaseColor.a;
    float emissive = 0.0;
    
    float4 viewPos = mul(CB_Viewport.viewMatrix, float4(input.WorldPos, 1.0));
    float depth = input.Position.z; //linearize_depth_log(viewPos.z, CB_Viewport.clipNearFar.x, CB_Viewport.clipNearFar.y);
    float depth2 = depth * depth;
    
    float coverage = clamp(opacity + lerp(MIN_COVERAGE, EMISSIVE_ELEMENT_COVERAGE, saturate(emissive)), MIN_COVERAGE, MAX_COVERAGE);
    float opticalDepth = -log(1.0 - coverage);
    float weight = opticalDepth;
    
    PS_MBOIT_STRUCT output;
    output.Moments_p1234 = float4(depth, depth2, depth * depth2, depth2 * depth2) * weight;
    output.OpticalDepth = opticalDepth;

    return output;
}

void resolve4DepthMoments(out float transmittance_at_depth, float depth, float zeroth_moment, float4 moments)
{
    transmittance_at_depth = 1;
	
    float b_0 = zeroth_moment;
    clip(b_0 - CLIP_THRESHOLD);

    float4 b_1234 = moments.xyzw;
    float2 b_even_q = b_1234.yw;
    float2 b_odd_q = b_1234.xz;

	// Dequantize the moments
    float2 b_even;
    float2 b_odd;
    offsetAndDequantizeMoments(b_even, b_odd, b_even_q, b_odd_q);
    const float4 bias_vector = float4(0, 0.628, 0, 0.628);
    const float moment_bias = 0.0006;
    const float overestimation = 0.0172;
	transmittance_at_depth = computeTransmittanceAtDepthFrom4PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
}

[[vk::location(0)]] float4 mboit_pass2_ps(PS_GBUFFER_STANDARD_INPUT input) : SV_TARGET0
{
    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(input, CB_Viewport, CB_Model, texture0, texture1, texture2, samplerState);
    
    float3 color = GBubfferStruct.BaseColor.rgb;
    float opacity = GBubfferStruct.BaseColor.a;
    float emissive = 0.0;
    
    float4 viewPos = mul(CB_Viewport.viewMatrix, float4(input.WorldPos, 1.0));
    float depth = input.Position.z; //linearize_depth_log(viewPos.z, CB_Viewport.clipNearFar.x, CB_Viewport.clipNearFar.y);
    
    float totalOpticalDepth = texture3.Sample(samplerState, input.UV).r;
    float4 totalMoments_p1234 = texture4.Sample(samplerState, input.UV);
    
    float transmittanceDepth = 0.0;
    resolve4DepthMoments(transmittanceDepth, depth, totalOpticalDepth, totalMoments_p1234);
    
    float coverage = clamp(opacity + lerp(MIN_COVERAGE, EMISSIVE_ELEMENT_COVERAGE, saturate(emissive)), MIN_COVERAGE, MAX_COVERAGE);
    
    return float4(color * transmittanceDepth, coverage * transmittanceDepth);
}

void resolveTotalMoments(out float total_transmittance, float zeroth_moment)
{
    total_transmittance = 1;
	
    float b_0 = zeroth_moment;
    clip(b_0 - CLIP_THRESHOLD);
    total_transmittance = exp(-b_0);
}

[[vk::location(0)]] float4 mboit_resolve_ps(PS_OFFSCREEN_INPUT input) : SV_TARGET0
{
    float3 baseColor = texture0.Sample(samplerState, input.UV).rgb;
    float4 totalTransparencyColor = texture1.Sample(samplerState, input.UV);
    
    float totalOpticalDepth = texture2.Sample(samplerState, input.UV).r;
    float totalTransmittance = 0.0;
    resolveTotalMoments(totalTransmittance, totalOpticalDepth);
    float f = (1.0 - totalTransmittance) / totalTransparencyColor.a;
    f = isfinite(f) ? f : 0.0;
    float3 transparencyColor = totalTransparencyColor.rgb * f;
    
    float4 finalColor = float4(baseColor + transparencyColor, totalTransmittance);
    return finalColor;
}