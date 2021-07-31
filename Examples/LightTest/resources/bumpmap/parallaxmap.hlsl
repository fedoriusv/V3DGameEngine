struct VS_INPUT
{
    float3 Position  : IN_POSITION;
    float3 Normal    : IN_NORMAL;
    float3 Tangent   : IN_TANGENT;
    float3 Bitangent : IN_BITANGENT;
    float2 UV        : IN_TEXTURE;
};

struct VS_UBO
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 normalMatrix;
    float4x4 viewMatrix;
};
ConstantBuffer<VS_UBO> vs_ubo : register(b0);

struct PS_INPUT
{
    float4 Pos       : SV_POSITION;

    float3 Position  : POSITION;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
    float2 UV        : TEXTURE;
};

PS_INPUT main_VS(VS_INPUT Input)
{
    PS_INPUT Output;
   
    float4 position = mul(vs_ubo.modelMatrix, float4(Input.Position, 1.0));
    Output.Pos = mul(vs_ubo.projectionMatrix, mul(vs_ubo.viewMatrix, position));
  
    Output.Position = position.xyz / position.w;
    Output.Normal = mul((float3x3)vs_ubo.normalMatrix, Input.Normal);
    Output.Tangent = mul((float3x3)vs_ubo.normalMatrix, Input.Tangent);
    Output.Bitangent = mul((float3x3)vs_ubo.normalMatrix, Input.Bitangent);
    Output.UV = Input.UV;

    return Output;
}

SamplerState samplerColor : register(s1);
Texture2D textureColor : register(t2);
SamplerState samplerFilter : register(s3);
Texture2D textureNormal : register(t4);
Texture2D textureHeight : register(t5);

struct LIGHT
{
    float4 position;
    float4 diffuse;
    float4 specular;
};

struct LIGHTS
{
    LIGHT lights[LIGHT_COUNT];
};
ConstantBuffer<LIGHTS> light : register(b6);

struct PS_UBO 
{
    float4 viewPosition;
};
ConstantBuffer<PS_UBO> ps_ubo : register(b7);

float4 main_PS(PS_INPUT Input) : SV_TARGET0
{
    return float4(0.0, 0.0, 0.0, 0.0);
}