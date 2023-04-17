struct VS_INPUT
{
    [[vk::location(0)]] float3 Position : POSITION;
};

struct VS_OUTPUT
{
    [[vk::location(0)]] float4 Pos : SV_POSITION;
};

struct VS_Buffer 
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
    float4x4 modelMatrix;
};
[[vk::binding(0, 0)]] ConstantBuffer<VS_Buffer> vs_buffer : register(b0, space0);

VS_OUTPUT main_VS(VS_INPUT input)
{
    float4 position = mul(vs_buffer.modelMatrix, float4(input.Position, 1.0));
    
    VS_OUTPUT output;
    output.Pos = mul(vs_buffer.projectionMatrix, mul(vs_buffer.viewMatrix, position));

    return output;
}

[[vk::location(0)]] float4 main_FS(VS_OUTPUT input) : SV_TARGET0
{
    return float4(1.0, 1.0, 1.0, 1.0);
}