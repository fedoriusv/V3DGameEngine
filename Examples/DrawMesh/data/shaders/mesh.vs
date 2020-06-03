struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXTURE;
};

struct UBO
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4   lightPos;
};

ConstantBuffer<UBO> ubo : register(b0);

struct PS_INPUT 
{
    float4 Position  : SV_POSITION;
    float3 Normal    : NORMAL;
    float2 UV        : TEXTURE;
    float3 View      : VIEW;
    float3 Light     : LIGHT;
};

PS_INPUT main(VS_INPUT Input)
{
    PS_INPUT Output;
   
    float4 position = mul(ubo.modelMatrix, float4(Input.Position, 1.0));
    float3 lightPosition = mul((float3x3)ubo.modelMatrix, ubo.lightPos.xyz);
    
    Output.Position = mul(ubo.viewMatrix, position);
    Output.Position = mul(ubo.projectionMatrix, Output.Position);
    Output.Normal = mul((float3x3)ubo.modelMatrix, Input.Normal);
    Output.UV = Input.UV;
    Output.Light = lightPosition - position.xyz;
    Output.View = -position.xyz;

    return Output;
}