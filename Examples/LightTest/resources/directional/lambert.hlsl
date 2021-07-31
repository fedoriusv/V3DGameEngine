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

    float3 Normal    : NORMAL;    
    float2 UV        : TEXTURE;
};

PS_INPUT main_VS(VS_INPUT Input)
{
    PS_INPUT Output;
   
    float4 position = mul(vs_ubo.modelMatrix, float4(Input.Position, 1.0));
    Output.Pos = mul(vs_ubo.projectionMatrix, mul(vs_ubo.viewMatrix, position));
    Output.Normal = mul((float3x3)vs_ubo.normalMatrix, Input.Normal);
    Output.UV = Input.UV;

    return Output;
}

SamplerState samplerColor : register(s1);
Texture2D textureColor : register(t2);

struct LIGHT
{
    float4 direction;
    float4 diffuse;
};

struct LIGHTS
{
    LIGHT lights[LIGHT_COUNT];
};
ConstantBuffer<LIGHTS> light : register(b3);

float4 main_PS(PS_INPUT Input) : SV_TARGET0
{
    float4 colorDiffuse = textureColor.Sample(samplerColor, Input.UV);
    float3 normal = normalize(Input.Normal);
    
    float4 OutColor = float4(0.0, 0.0, 0.0, 1.0);
    for (int l = 0; l < LIGHT_COUNT; ++l)
    {
        float3 lightDir = normalize(light.lights[l].direction.xyz);
        OutColor += colorDiffuse * max(dot(normal, lightDir), 0.01) * light.lights[l].diffuse;
    }

    return OutColor;
}