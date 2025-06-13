#ifndef _OFFSCREEN_COMMON_HLSL_
#define _OFFSCREEN_COMMON_HLSL_

///////////////////////////////////////////////////////////////////////////////////////

struct VS_OFFSCREEN_OUTPUT
{
    float4                     Position : SV_POSITION;
    [[vk::location(0)]] float2 UV       : TEXTURE;
};

typedef VS_OFFSCREEN_OUTPUT PS_OFFSCREEN_INPUT;

///////////////////////////////////////////////////////////////////////////////////////

VS_OFFSCREEN_OUTPUT _offscreen_vs(uint VertexID)
{
    VS_OFFSCREEN_OUTPUT Output;

    float2 texcoord = float2((VertexID << 1) & 2, VertexID & 2);
    Output.UV = texcoord;
    Output.Position = float4(texcoord.x * 2.0f - 1.0f, -(texcoord.y * 2.0f - 1.0f), 0.0f, 1.0f);

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_OFFSCREEN_COMMON_HLSL_