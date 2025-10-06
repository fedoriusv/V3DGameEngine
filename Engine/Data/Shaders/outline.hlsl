#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport    : register(b0, space0);

struct OutlineBuffer
{
    float4 lineColor;
    float  lineThickness;
};

[[vk::binding(1, 1)]] ConstantBuffer<OutlineBuffer> cb_Outline : register(b0, space1);
[[vk::binding(2, 1)]] SamplerState s_SamplerState              : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D t_ColorTextrue                 : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D t_MaterialTexture              : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D<float> t_SelectionTexture      : register(t2, space1);
[[vk::binding(6, 1)]] RWStructuredBuffer<uint> rw_BufferID     : register(u0, space1);

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 main_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float4 fragColor = t_ColorTextrue.Sample(s_SamplerState, Input.UV);
    
    //readback object ID
    uint2 pixelScreeSpace = uint2(Input.Position.xy);
    uint2 cursorPosition = uint2(cb_Viewport.cursorPosition);
    if (cursorPosition.x == pixelScreeSpace.x && cursorPosition.y == pixelScreeSpace.y)
    {
        float centerID = t_MaterialTexture.Sample(s_SamplerState, Input.UV).b;
        uint objectID = uint(centerID);
        
        uint originalValue;
        InterlockedExchange(rw_BufferID[0], objectID, originalValue);
    }
    
    //outline
    uint selectedRegion = (uint) t_SelectionTexture.Sample(s_SamplerState, Input.UV).r;
    if (selectedRegion != 0)
    {
        float2 thicknessOffset = float2(1.0 / cb_Viewport.viewportSize.x, 1.0 / cb_Viewport.viewportSize.y) * cb_Outline.lineThickness;
        float idLeft = t_SelectionTexture.Sample(s_SamplerState, Input.UV + float2(-thicknessOffset.x, 0)).r;
        float idRight = t_SelectionTexture.Sample(s_SamplerState, Input.UV + float2(thicknessOffset.x, 0)).r;
        float idUp = t_SelectionTexture.Sample(s_SamplerState, Input.UV + float2(0, -thicknessOffset.y)).r;
        float idDown = t_SelectionTexture.Sample(s_SamplerState, Input.UV + float2(0, thicknessOffset.y)).r;
        
        float edge = 
            step(0.01, abs(selectedRegion - idLeft)) +
            step(0.01, abs(selectedRegion - idRight)) +
            step(0.01, abs(selectedRegion - idUp)) +
            step(0.01, abs(selectedRegion - idDown));
        
        if (edge > 0.0)
        {
            fragColor = cb_Outline.lineColor;
        }
    }
    
    return fragColor;
}

///////////////////////////////////////////////////////////////////////////////////////