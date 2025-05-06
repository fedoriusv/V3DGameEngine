#ifndef _GLOBAL_HLSL_
#define _GLOBAL_HLSL_

struct Viewport
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
	float4   cameraPosition;
	float4   viewportSize;
};

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> viewport : register(b0, space0);


struct LIGHT
{
    float4 lightPosition;
    float4 color;
};

[[vk::binding(1, 0)]] ConstantBuffer<LIGHT> light : register(b1, space0);

#endif //_GLOBAL_HLSL_