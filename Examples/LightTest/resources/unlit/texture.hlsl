struct VS_INPUT
{
    float3 Position  : IN_POSITION;
    float3 Normal    : IN_NORMAL;
    float3 Tangent   : IN_TANGENT;
    float3 Bitangent : IN_BITANGENT;     
    float2 UV        : IN_TEXTURE;
};

struct UBO
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;  
    float4x4 viewMatrix;
};
ConstantBuffer<UBO> ubo : register(b0);

struct PS_INPUT
{
    float4 Pos       : SV_POSITION;
    float2 UV        : TEXTURE;
};

SamplerState samplerColor : register(s1);
Texture2D textureColor : register(t2);

PS_INPUT main_VS(VS_INPUT Input)
{
    PS_INPUT Output;
   
    float4 position = mul(ubo.modelMatrix, float4(Input.Position, 1.0));
    Output.Pos = mul(ubo.projectionMatrix, mul(ubo.viewMatrix, position));
    Output.UV = Input.UV;

    return Output;
}

float4 main_PS(PS_INPUT Input) : SV_TARGET0
{
    float4 OutColor = textureColor.Sample(samplerColor, Input.UV);
    return OutColor;
}