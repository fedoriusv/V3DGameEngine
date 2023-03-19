struct VS_OUTPUT
{
    [[vk::location(0)]] float4 Pos : SV_Position;
    [[vk::location(1)]] float2 UV  : TEXTURE;
};

VS_OUTPUT main_VS(uint vertexID : SV_VertexID)
{
    VS_OUTPUT output;
    
    output.UV = float2((vertexID << 1) & 2, vertexID & 2);
    output.Pos = float4(output.UV.x * 2.0f - 1.0f, -(output.UV.y * 2.0f - 1.0f), 0.0f, 1.0f);
    
    return output;
}

[[vk::binding(0, 0)]] SamplerState colorSampler : register(s0, space0);
[[vk::binding(1, 0)]] Texture2D colorTexture : register(t0, space0);

[[vk::location(0)]] float4 main_FS(VS_OUTPUT input) : SV_TARGET0
{
    float4 outFragColor = colorTexture.SampleLevel(colorSampler, input.UV, 0);
    return outFragColor;
}