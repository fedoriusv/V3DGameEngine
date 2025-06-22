#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);

struct OutlineBuffer
{
    float4 lineColor;
    float  lineThickness;
    uint   selectedID;
};
[[vk::binding(1, 1)]] ConstantBuffer<OutlineBuffer> CB_Ountline : register(b0, space1);
[[vk::binding(2, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D textureColor : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D textureMaterial : register(t1, space1);
[[vk::binding(5, 1)]] RWStructuredBuffer<uint> rwBufferID : register(u0, space1);

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 main_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float4 fragColor = textureColor.Sample(samplerState, Input.UV);
    
    float centerID = textureMaterial.Sample(samplerState, Input.UV).b;
    uint objectID = uint(centerID);
    
    //readback object ID
    uint2 pixelScreeSpace = uint2(Input.Position.xy);
    uint2 cursorPosition = uint2(CB_Viewport.cursorPosition);
    if (cursorPosition.x == pixelScreeSpace.x && cursorPosition.y == pixelScreeSpace.y)
    {
        uint originalValue;
        InterlockedExchange(rwBufferID[0], objectID, originalValue);
    }
    
    //outline
    uint selectedID = CB_Ountline.selectedID;
    if (selectedID != 0 && selectedID == objectID)
    {
        float2 thicknessOffset = float2(1.0 / CB_Viewport.viewportSize.x, 1.0 / CB_Viewport.viewportSize.y) * CB_Ountline.lineThickness;
        float idLeft = textureMaterial.Sample(samplerState, Input.UV + float2(-thicknessOffset.x, 0)).b;
        float idRight = textureMaterial.Sample(samplerState, Input.UV + float2(thicknessOffset.x, 0)).b;
        float idUp = textureMaterial.Sample(samplerState, Input.UV + float2(0, -thicknessOffset.y)).b;
        float idDown = textureMaterial.Sample(samplerState, Input.UV + float2(0, thicknessOffset.y)).b;
        
        float edge = 
            step(0.01, abs(centerID - idLeft)) +
            step(0.01, abs(centerID - idRight)) +
            step(0.01, abs(centerID - idUp)) +
            step(0.01, abs(centerID - idDown));
        
        if (edge > 0.0)
        {
            fragColor = CB_Ountline.lineColor;
        }
    }
    
    return fragColor;
}