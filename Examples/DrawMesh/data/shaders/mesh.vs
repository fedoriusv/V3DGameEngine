struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXTURE;
};

struct UBO
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

ConstantBuffer<UBO> ubo : register(b0);

struct PS_INPUT 
{
    float4 Pos       : SV_POSITION;
    
    float3 Position  : POSITION;
    float3 Normal    : NORMAL;
    float2 UV        : TEXTURE;
};

PS_INPUT main(VS_INPUT Input)
{
    PS_INPUT Output;
   
    float4 position = mul(ubo.modelMatrix, float4(Input.Position, 1.0));
    
    Output.Pos = mul(ubo.projectionMatrix, mul(ubo.viewMatrix, position));
    Output.Position = position.xyz / position.w;
    Output.Normal = mul((float3x3)ubo.normalMatrix, Input.Normal);
    Output.UV = Input.UV;

    return Output;
}