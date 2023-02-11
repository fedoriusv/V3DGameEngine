struct VS_OUTPUT
{
    float4 Pos : SV_Position;
    float2 UV : TEXTURE;
};

VS_OUTPUT main_VS(uint vertexID : SV_VertexID)
{
    VS_OUTPUT output;
    
    output.UV = float2((vertexID << 1) & 2, vertexID & 2);
    output.Pos = float4(output.UV.x * 2.0f - 1.0f, -(output.UV.y * 2.0f - 1.0f), 0.0f, 1.0f);
    
    return output;
}

SamplerState colorSampler : register(s0, space0);
Texture2D colorTexture : register(t1, space0);

float4 main_FS(VS_OUTPUT input) : SV_TARGET0
{
    float4 outFragColor = colorTexture.SampleLevel(colorSampler, input.UV, 0);
    return outFragColor;
}