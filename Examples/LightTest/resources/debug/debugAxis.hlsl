struct VS_UBO
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;  
    float4x4 viewMatrix;
};
ConstantBuffer<VS_UBO> vs_ubo : register(b0);

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
};

PS_INPUT main_VS(uint VertexID : SV_VertexID)
{
    PS_INPUT Output;
   
    float4 position = mul(vs_ubo.modelMatrix, float4(VertexID * 1.0, 0.0, 0.0, 1.0));
    Output.Pos = mul(vs_ubo.projectionMatrix, mul(vs_ubo.viewMatrix, position));

    return Output;
}


struct PS_UBO
{
    float4 color;
};
ConstantBuffer<PS_UBO> debugColor : register(b1);

float4 main_PS(PS_INPUT Input) : SV_TARGET0
{
    float4 OutColor = debugColor.color;
    return OutColor;
}