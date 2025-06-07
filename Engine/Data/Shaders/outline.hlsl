#include "global.hlsli"

struct PS_INPUT
{
    float4                     Position : SV_POSITION;
    [[vk::location(0)]] float2 UV       : TEXTURE;
};

struct OutlineBuffer
{
    float4 slectedID;
};
[[vk::binding(1, 1)]] ConstantBuffer<OutlineBuffer> CB_Ountline : register(b0, space1);

[[vk::binding(2, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D textureColor : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D textureMaterial : register(t1, space1);
[[vk::binding(5, 1)]] RWStructuredBuffer<uint> rwBufferID : register(u0, space1);

float4 main_ps(PS_INPUT Input) : SV_TARGET0
{
    float4 fragColor = textureColor.Sample(samplerState, Input.UV);
    uint objectID = uint(textureMaterial.Sample(samplerState, Input.UV).b);
    uint slectedID = uint(CB_Ountline.slectedID.x);
    if (slectedID != 0 && slectedID == objectID)
    {
        fragColor += float4(0.5, 0.5, 0.5, 0.0);
    }
    
    uint2 pixelScreeSpace = uint2(floor(Input.Position.xy + 0.5));
    uint2 cursorPosition = uint2(viewport.cursorPosition);
    if (cursorPosition.x == pixelScreeSpace.x && cursorPosition.y == pixelScreeSpace.y)
    {
        uint originalValue;
        InterlockedExchange(rwBufferID[0], objectID, originalValue);
    }

    return fragColor;
}