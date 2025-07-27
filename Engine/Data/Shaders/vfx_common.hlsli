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
    const float scale = 0.02f;
    
    // Define quad corners based on vertexID
    const float2 quadVerts[4] =
    {
        float2(-scale, scale), // top-left
        float2(scale, scale), // top-right
        float2(-scale, -scale), // bottom-left
        float2(scale, -scale), // bottom-right
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
    float3 wordPos = float3(Model.modelMatrix[0][3], Model.modelMatrix[1][3], Model.modelMatrix[2][3]);
    float3 position = wordPos + (right * offset.x) + (up * offset.y);
    
    //// Apply scaling from model matrix (scale encoded in basis vectors)
    //float3 scaledOffset =
    //    offsetWorld.x * float3(Model.modelMatrix._11, Model.modelMatrix._21, Model.modelMatrix._31) +
    //    offsetWorld.y * float3(Model.modelMatrix._12, Model.modelMatrix._22, Model.modelMatrix._32) +
    //    offsetWorld.z * float3(Model.modelMatrix._13, Model.modelMatrix._23, Model.modelMatrix._33);
    
    //// Translate to final world position
    //float3 position = float3(Model.modelMatrix._41, Model.modelMatrix._42, Model.modelMatrix._43) + scaledOffset;

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