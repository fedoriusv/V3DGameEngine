struct VS_INPUT
{
    [[vk::location(0)]] float3 Position : POSITION;
    [[vk::location(1)]] float3 Normal   : NORMAL;
    [[vk::location(2)]] float2 UV       : TEXTURE;
};

struct VS_OUTPUT
{
    [[vk::location(0)]] float4 Pos      : SV_POSITION;
    [[vk::location(1)]] float3 Normal   : NORMAL;
    [[vk::location(2)]] float2 UV       : TEXTURE;
};

struct VS_Buffer 
{
    matrix projectionMatrix;
    matrix viewMatrix;
    matrix modelMatrix;
};
[[vk::binding(0, 0)]] ConstantBuffer<VS_Buffer> vs_buffer : register(b0, space0);

VS_OUTPUT main_VS(VS_INPUT input)
{
    float4 position = mul(vs_buffer.modelMatrix, float4(input.Position, 1.0));
    
    VS_OUTPUT output;
    output.UV = input.UV;
    output.Normal = input.Normal;
    output.Pos = mul(vs_buffer.projectionMatrix, mul(vs_buffer.viewMatrix, position));

    return output;
}

[[vk::binding(1, 0)]] SamplerState colorSampler : register(s0, space0);
[[vk::binding(2, 0)]] Texture2D colorTexture : register(t0, space0);

[[vk::location(0)]] float4 main_FS(VS_OUTPUT input) : SV_TARGET0
{
    float4 outFragColor = colorTexture.Sample(colorSampler, input.UV);
    return outFragColor;
}