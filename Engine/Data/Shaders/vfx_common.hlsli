#ifndef _VFX_COMMON_HLSL_
#define _VFX_COMMON_HLSL_

#include "global.hlsli"
#include "viewport.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

VS_SIMPLE_OUTPUT _billboard_vs(
    in uniform ConstantBuffer<Viewport> Viewport,
    in uniform ConstantBuffer<ModelBuffer> Model,
    uint VertexID)
{
    // Define quad corners based on vertexID
    const float2 quadVerts[4] =
    {
        float2(-0.5f, 0.5f),    // top-left
        float2(0.5f, 0.5f),     // top-right
        float2(-0.5f, -0.5f),   // bottom-left
        float2(0.5f, -0.5f),    // bottom-right
    };

    const float2 uvCoords[4] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 1.0f),
    };
    
    float3 right = float3(Viewport.viewMatrix[0][0], Viewport.viewMatrix[1][0], Viewport.viewMatrix[2][0]); // camera right
    float3 up = float3(Viewport.viewMatrix[0][1], Viewport.viewMatrix[1][1], Viewport.viewMatrix[2][1]); // camera up
    
    float2 offset = quadVerts[VertexID];
    float3 wordPos = float3(Model.modelMatrix[0][3], Model.modelMatrix[1][3], Model.modelMatrix[1][3]);
    float3 position = wordPos + (right * offset.x) + (up * offset.y);
    
    VS_SIMPLE_OUTPUT Output;
    
    Output.ClipPos = mul(Viewport.projectionMatrix, mul(Viewport.viewMatrix, float4(position, 1.0)));
    Output.WorldPos = position;
    Output.Normal = float3(0.0, 1.0, 0.0);
    Output.UV = uvCoords[VertexID];
    
    Output.Position = Output.ClipPos;
    
    return Output;

}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_VFX_COMMON_HLSL_