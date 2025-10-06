#ifndef _VFX_COMMON_HLSL_
#define _VFX_COMMON_HLSL_

#include "global.hlsli"
#include "viewport.hlsli"

///////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////

VS_SIMPLE_OUTPUT _horizontal_billboard_vs(
    in uniform ConstantBuffer<Viewport> Viewport,
    in uniform ConstantBuffer<ModelBuffer> Model,
    uint VertexID)
{
    const float scale = 0.02f;
    
    // Define quad corners based on vertexID
    const float2 quadVerts[4] =
    {
        float2(-scale, scale),  // top-left
        float2(scale, scale),   // top-right
        float2(-scale, -scale), // bottom-left
        float2(scale, -scale),  // bottom-right
    };

    const float2 uvCoords[4] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 1.0f),
    };
    
    // Extract camera forward and right vectors in world space
    float3 forward = normalize(float3(Viewport.viewMatrix._31, 0.0f, Viewport.viewMatrix._33)); // Z-axis (ignore Y)
    float3 right = normalize(float3(Viewport.viewMatrix._11, 0.0f, Viewport.viewMatrix._13)); // X-axis (ignore Y)
    float3 up = float3(0.0f, 1.0f, 0.0f); // Fixed Y-up
    
    float2 offset = quadVerts[VertexID];
    float3 worldPos = float3(Model.modelMatrix[0][3], Model.modelMatrix[1][3], Model.modelMatrix[2][3]);
    float3 position = worldPos + (right * offset.x) + (up * offset.y);
    
    VS_SIMPLE_OUTPUT Output;
    
    Output.ClipPos = mul(Viewport.projectionMatrix, mul(Viewport.viewMatrix, float4(position, 1.0)));
    Output.WorldPos = position;
    Output.Normal = float3(0.0, 1.0, 0.0);
    Output.UV = uvCoords[VertexID];
    
    Output.Position = Output.ClipPos;
    
    return Output;

}

VS_SIMPLE_OUTPUT _billboard_vs(
    in uniform ConstantBuffer<Viewport> Viewport,
    in uniform ConstantBuffer<ModelBuffer> Model,
    uint VertexID)
{
    // Define quad corners based on vertexID
    const float2 quadVerts[4] =
    {
        float2(-0.5, 0.5),  // top-left
        float2(0.5, 0.5),   // top-right
        float2(-0.5, -0.5), // bottom-left
        float2(0.5, -0.5),  // bottom-right
    };

    const float2 uvCoords[4] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 1.0f),
    };
    
    float3 scale = float3(0.1f, 0.1f, 0.1f);
    //scale.x = length(Model.modelMatrix[0].xyz);
    //scale.y = length(Model.modelMatrix[1].xyz);
    //scale.z = length(Model.modelMatrix[2].xyz);
    

    float3 right = normalize(float3(Viewport.viewMatrix[0].xyz));
    float3 up = normalize(float3(Viewport.viewMatrix[1].xyz));
    
    float2 offset = quadVerts[VertexID] * float2(scale.x, scale.y);
    float3 worldPos = float3(Model.modelMatrix[0][3], Model.modelMatrix[1][3], Model.modelMatrix[2][3]);
    float3 worldOffset = offset.x * right + offset.y * up;
    float3 position = worldPos + worldOffset;

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