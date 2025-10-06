#include "global.hlsli"
#include "viewport.hlsli"
#include "gbuffer_common.hlsli"
#include "offscreen_common.hlsli"

#define SINGLE_PRECISION 1
#define NUM_MOMENTS 4
#define TRIGONOMETRIC 0

#include "MomentMath.hlsli"

/*! This functions relies on fixed function additive blending to compute the 
	vector of moments.moment vector. The shader that calls this function must 
	provide the required render targets.*/
#if NUM_MOMENTS == 4
void generateMoments(float depth, float transmittance, float4 wrapping_zone_parameters, out float b_0, out float4 b)
#elif NUM_MOMENTS == 6
#if USE_R_RG_RBBA_FOR_MBOIT6
void generateMoments(float depth, float transmittance, float4 wrapping_zone_parameters, out float b_0, out float2 b_12, out float4 b_3456)
#else
void generateMoments(float depth, float transmittance, float4 wrapping_zone_parameters, out float b_0, out float2 b_12, out float2 b_34, out float2 b_56)
#endif
#elif NUM_MOMENTS == 8
void generateMoments(float depth, float transmittance, float4 wrapping_zone_parameters, out float b_0, out float4 b_even, out float4 b_odd)
#endif
{
    float absorbance = -log(transmittance);

    b_0 = absorbance;
#if TRIGONOMETRIC
	float phase = mad(depth, wrapping_zone_parameters.y, wrapping_zone_parameters.y);
	float2 circle_point;
	sincos(phase, circle_point.y, circle_point.x);
	float2 circle_point_pow2 = Multiply(circle_point, circle_point);
#if NUM_MOMENTS == 4
	b = float4(circle_point, circle_point_pow2) * absorbance;
#elif NUM_MOMENTS == 6
	b_12 = circle_point * absorbance;
#if USE_R_RG_RBBA_FOR_MBOIT6
	b_3456 = float4(circle_point_pow2, Multiply(circle_point, circle_point_pow2)) * absorbance;
#else
	b_34 = circle_point_pow2 * absorbance;
	b_56 = Multiply(circle_point, circle_point_pow2) * absorbance;
#endif
#elif NUM_MOMENTS == 8
	b_even = float4(circle_point_pow2, Multiply(circle_point_pow2, circle_point_pow2)) * absorbance;
	b_odd = float4(circle_point, Multiply(circle_point, circle_point_pow2)) * absorbance;
#endif
#else
    float depth_pow2 = depth * depth;
    float depth_pow4 = depth_pow2 * depth_pow2;
#if NUM_MOMENTS == 4
    b = float4(depth, depth_pow2, depth_pow2 * depth, depth_pow4) * absorbance;
#elif NUM_MOMENTS == 6
	b_12 = float2(depth, depth_pow2) * absorbance;
#if USE_R_RG_RBBA_FOR_MBOIT6
	b_3456 = float4(depth_pow2 * depth, depth_pow4, depth_pow4 * depth, depth_pow4 * depth_pow2) * absorbance;
#else
	b_34 = float2(depth_pow2 * depth, depth_pow4) * absorbance;
	b_56 = float2(depth_pow4 * depth, depth_pow4 * depth_pow2) * absorbance;
#endif
#elif NUM_MOMENTS == 8
	float depth_pow6 = depth_pow4 * depth_pow2;
	b_even = float4(depth_pow2, depth_pow4, depth_pow6, depth_pow6 * depth_pow2) * absorbance;
	b_odd = float4(depth, depth_pow2 * depth, depth_pow4 * depth, depth_pow6 * depth) * absorbance;
#endif
#endif
}

/*! This function is to be called from the shader that composites the 
	transparent fragments. It reads the moments and calls the appropriate 
	function to reconstruct the transmittance at the specified depth.*/
void resolveMoments(out float transmittance_at_depth, out float total_transmittance, float depth, float2 sv_pos, float zeroth_moment, float4 moments, float moment_bias, float overestimation)
{
    int4 idx0 = int4(sv_pos, 0, 0);
    int4 idx1 = idx0;
    idx1[2] = 1;

    transmittance_at_depth = 1;
    total_transmittance = 1;
	
    float b_0 = zeroth_moment;
    clip(b_0 - 0.00100050033f);
    total_transmittance = exp(-b_0);

#if NUM_MOMENTS == 4
#if TRIGONOMETRIC
	float4 b_tmp = moments.Load(idx0);
	float2 trig_b[2];
	trig_b[0] = b_tmp.xy;
	trig_b[1] = b_tmp.zw;
#if SINGLE_PRECISION
	trig_b[0] /= b_0;
	trig_b[1] /= b_0;
#else
	trig_b[0] = mad(trig_b[0], 2.0, -1.0);
	trig_b[1] = mad(trig_b[1], 2.0, -1.0);
#endif
	transmittance_at_depth = computeTransmittanceAtDepthFrom2TrigonometricMoments(b_0, trig_b, depth,  moment_bias,  overestimation, wrapping_zone_parameters);
#else
    float4 b_1234 = moments;
#if SINGLE_PRECISION
    float2 b_even = b_1234.yw;
    float2 b_odd = b_1234.xz;

    b_even /= b_0;
    b_odd /= b_0;

    const float4 bias_vector = float4(0, 0.375, 0, 0.375);
#else
	float2 b_even_q = b_1234.yw;
	float2 b_odd_q = b_1234.xz;

	// Dequantize the moments
	float2 b_even;
	float2 b_odd;
	offsetAndDequantizeMoments(b_even, b_odd, b_even_q, b_odd_q);
	const float4 bias_vector = float4(0, 0.628, 0, 0.628);
#endif
    transmittance_at_depth = computeTransmittanceAtDepthFrom4PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
#endif
#elif NUM_MOMENTS == 6
	int4 idx2 = idx0;
	idx2[2] = 2;
#if TRIGONOMETRIC
	float2 trig_b[3];
	trig_b[0] = moments.Load(idx0).xy;
#if USE_R_RG_RBBA_FOR_MBOIT6
	float4 tmp = extra_moments.Load(idx0);
	trig_b[1] = tmp.xy;
	trig_b[2] = tmp.zw;
#else
	trig_b[1] = moments.Load(idx1).xy;
	trig_b[2] = moments.Load(idx2).xy;
#endif
#if SINGLE_PRECISION
	trig_b[0] /= b_0;
	trig_b[1] /= b_0;
	trig_b[2] /= b_0;
#else
	trig_b[0] = mad(trig_b[0], 2.0, -1.0);
	trig_b[1] = mad(trig_b[1], 2.0, -1.0);
	trig_b[2] = mad(trig_b[2], 2.0, -1.0);
#endif
	transmittance_at_depth = computeTransmittanceAtDepthFrom3TrigonometricMoments(b_0, trig_b, depth, moment_bias, overestimation, wrapping_zone_parameters);
#else
	float2 b_12 = moments.Load(idx0).xy;
#if USE_R_RG_RBBA_FOR_MBOIT6
	float4 tmp = extra_moments.Load(idx0);
	float2 b_34 = tmp.xy;
	float2 b_56 = tmp.zw;
#else
	float2 b_34 = moments.Load(idx1).xy;
	float2 b_56 = moments.Load(idx2).xy;
#endif
#if SINGLE_PRECISION
	float3 b_even = float3(b_12.y, b_34.y, b_56.y);
	float3 b_odd = float3(b_12.x, b_34.x, b_56.x);

	b_even /= b_0;
	b_odd /= b_0;

	const float bias_vector[6] = { 0, 0.48, 0, 0.451, 0, 0.45 };
#else
	float3 b_even_q = float3(b_12.y, b_34.y, b_56.y);
	float3 b_odd_q = float3(b_12.x, b_34.x, b_56.x);
	// Dequantize b_0 and the other moments
	float3 b_even;
	float3 b_odd;
	offsetAndDequantizeMoments(b_even, b_odd, b_even_q, b_odd_q);

	const float bias_vector[6] = { 0, 0.5566, 0, 0.489, 0, 0.47869382 };
#endif
	transmittance_at_depth = computeTransmittanceAtDepthFrom6PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
#endif
#elif NUM_MOMENTS == 8
#if TRIGONOMETRIC
	float4 b_tmp = moments.Load(idx0);
	float4 b_tmp2 = moments.Load(idx1);
#if SINGLE_PRECISION
	float2 trig_b[4] = {
		b_tmp2.xy / b_0,
		b_tmp.xy / b_0,
		b_tmp2.zw / b_0,
		b_tmp.zw / b_0
	};
#else
	float2 trig_b[4] = {
		mad(b_tmp2.xy, 2.0, -1.0),
		mad(b_tmp.xy, 2.0, -1.0),
		mad(b_tmp2.zw, 2.0, -1.0),
		mad(b_tmp.zw, 2.0, -1.0)
	};
#endif
	transmittance_at_depth = computeTransmittanceAtDepthFrom4TrigonometricMoments(b_0, trig_b, depth, moment_bias, overestimation, wrapping_zone_parameters);
#else
#if SINGLE_PRECISION
	float4 b_even = moments.Load(idx0);
	float4 b_odd = moments.Load(idx1);

	b_even /= b_0;
	b_odd /= b_0;
	const float bias_vector[8] = { 0, 0.75, 0, 0.67666666666666664, 0, 0.63, 0, 0.60030303030303034 };
#else
	float4 b_even_q = moments.Load(idx0);
	float4 b_odd_q = moments.Load(idx1);

	// Dequantize the moments
	float4 b_even;
	float4 b_odd;
	offsetAndDequantizeMoments(b_even, b_odd, b_even_q, b_odd_q);
	const float bias_vector[8] = { 0, 0.42474916387959866, 0, 0.22407802675585284, 0, 0.15369230769230768, 0, 0.12900440529089119 };
#endif
	transmittance_at_depth = computeTransmittanceAtDepthFrom8PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> cb_Model : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState s_SamplerState          : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D t_Texture0                 : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D t_Texture1                 : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D t_Texture2                 : register(t2, space1);
[[vk::binding(6, 1)]] Texture2D t_Texture3                 : register(t3, space1);
[[vk::binding(7, 1)]] Texture2D t_Texture4                 : register(t4, space1);

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

PS_MBOIT_STRUCT mboit_pass1_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    float4 baseColor = t_Texture0.Sample(s_SamplerState, Input.UV);
    float3 albedo = srgb_linear(baseColor.rgb);
    float3 normal = t_Texture1.Sample(s_SamplerState, Input.UV).rgb * 2.0 - 1.0;
    float3 materials = t_Texture2.Sample(s_SamplerState, Input.UV).rgb;
    float roughness = materials.r;
    float metalness = materials.g;
	
    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(Input, cb_Viewport, cb_Model, albedo, baseColor.a, normal, roughness, metalness);
    
    float3 color = GBubfferStruct.BaseColor.rgb;
    float opacity = GBubfferStruct.BaseColor.a;
    float emissive = 0.0;
    
    float4 viewPos = mul(cb_Viewport.viewMatrix, float4(Input.WorldPos, 1.0));
    float depth = linearize_depth_log(viewPos.z, cb_Viewport.clipNearFar.x, cb_Viewport.clipNearFar.y);
    
    float coverage = clamp(opacity + lerp(MIN_COVERAGE, EMISSIVE_ELEMENT_COVERAGE, saturate(emissive)), MIN_COVERAGE, MAX_COVERAGE);
    float transmittance = 1.0 - coverage;

    PS_MBOIT_STRUCT output;
    generateMoments(depth, transmittance, float4(0.0, 0.0, 0.0, 0.0), output.OpticalDepth, output.Moments_p1234);

    return output;
}


struct PS_TRANSPARENCY_OUTPUT
{
    [[vk::location(0)]] float4 Accumulate : COLOR;
    [[vk::location(1)]] float4 Material : MATERIAL;
    [[vk::location(2)]] float2 Velocity : VELOCITY;
};

PS_TRANSPARENCY_OUTPUT mboit_pass2_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    float4 baseColor = t_Texture0.Sample(s_SamplerState, Input.UV);
    float3 albedo = srgb_linear(baseColor.rgb);
    float3 normal = t_Texture1.Sample(s_SamplerState, Input.UV).rgb * 2.0 - 1.0;
    float3 materials = t_Texture2.Sample(s_SamplerState, Input.UV).rgb;
    float roughness = materials.r;
    float metalness = materials.g;
	
    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(Input, cb_Viewport, cb_Model, albedo, baseColor.a, normal, roughness, metalness);
    
    float3 color = GBubfferStruct.BaseColor.rgb;
    float opacity = GBubfferStruct.BaseColor.a;
    float emissive = 0.0;
    
    float4 viewPos = mul(cb_Viewport.viewMatrix, float4(Input.WorldPos, 1.0));
    float depth = linearize_depth_log(viewPos.z, cb_Viewport.clipNearFar.x, cb_Viewport.clipNearFar.y);
    
    float2 positionScreenUV = Input.Position.xy * (1.0 / cb_Viewport.viewportSize.xy);
    float zeroMoment = t_Texture3.Sample(s_SamplerState, positionScreenUV).r;
    float4 totalMoments_p1234 = t_Texture4.Sample(s_SamplerState, positionScreenUV);
    
    float transmittanceDepth = 0.0;
    float totalTransmittance = 0.0;
    const float moment_bias = 0.0006;
    const float overestimation = 0.0172;
    resolveMoments(transmittanceDepth, totalTransmittance, depth, viewPos.xy, zeroMoment, totalMoments_p1234, moment_bias, overestimation);
    
    float coverage = clamp(opacity + lerp(MIN_COVERAGE, EMISSIVE_ELEMENT_COVERAGE, saturate(emissive)), MIN_COVERAGE, MAX_COVERAGE);

    PS_TRANSPARENCY_OUTPUT Output;
    Output.Accumulate = float4(color * opacity * transmittanceDepth, coverage * transmittanceDepth);
    Output.Material = GBubfferStruct.Material;
    Output.Velocity = GBubfferStruct.Velocity;
	
    return Output;
}

void resolveTotalMoments(out float total_transmittance, float zeroth_moment)
{
    total_transmittance = 1;
	
    float b_0 = zeroth_moment;
    clip(b_0 - CLIP_THRESHOLD);
    total_transmittance = exp(-b_0);
}

[[vk::location(0)]] float4 mboit_resolve_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float3 baseColor = t_Texture0.SampleLevel(s_SamplerState, Input.UV, 0).rgb;
    float4 totalTransparencyColor = t_Texture1.SampleLevel(s_SamplerState, Input.UV, 0);
    
    float totalOpticalDepth = t_Texture2.SampleLevel(s_SamplerState, Input.UV, 0).r;
    float totalTransmittance = 0.0;
    resolveTotalMoments(totalTransmittance, totalOpticalDepth);
    float f = (1.0 - totalTransmittance) / totalTransparencyColor.a;
    f = isfinite(f) ? f : 0.0;
    float3 transparencyColor = totalTransparencyColor.rgb * f;

    float4 finalColor = float4(baseColor * totalTransmittance + transparencyColor, 1.0);
    return finalColor;
}