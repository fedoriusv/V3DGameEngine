struct VS_INPUT
{
    [[vk::location(0)]] float3 Position : IN_POSITION;
    [[vk::location(1)]] float3 Normal   : IN_NORMAL;
    [[vk::location(2)]] float2 UV       : IN_TEXTURE;
};

struct UBO
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

[[vk::binding(0, 0)]] ConstantBuffer<UBO> ubo : register(b0);

struct PS_INPUT
{
    [[vk::location(0)]] float4 Pos       : SV_POSITION;
    [[vk::location(1)]] float3 Position  : POSITION;
    [[vk::location(2)]] float3 Normal    : NORMAL;
    [[vk::location(3)]] float2 UV        : TEXTURE;
};

PS_INPUT main_vs(VS_INPUT Input)
{
    PS_INPUT Output;
   
    float4 position = mul(ubo.modelMatrix, float4(Input.Position, 1.0));
    
    Output.Pos = mul(ubo.projectionMatrix, mul(ubo.viewMatrix, position));
    Output.Position = position.xyz / position.w;
    Output.Normal = mul((float3x3)ubo.normalMatrix, Input.Normal);
    Output.UV = Input.UV;

    return Output;
}

[[vk::binding(1, 0)]] SamplerState samplerColor : register(s0);
[[vk::binding(2, 0)]] Texture2D textureColor : register(t0);

struct LIGHT
{
    float4 lightPosition;
};

[[vk::binding(3, 0)]] ConstantBuffer<LIGHT> light : register(b1);

[[vk::location(0)]] float4 main_ps(PS_INPUT Input) : SV_TARGET0
{
    float4 OutColor = textureColor.Sample(samplerColor, Input.UV);

    float3 N = normalize(Input.Normal);
    float3 L = normalize(light.lightPosition.xyz - Input.Position);
    float diffuseKoeff = max(dot(N, L), 0.01);
    OutColor = float4(diffuseKoeff * OutColor.rgb, 1.0);
    
    return OutColor;
}