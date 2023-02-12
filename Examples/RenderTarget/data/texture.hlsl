struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXTURE;
};

struct VS_OUTPUT
{
    float4 Pos          : SV_POSITION;
    float3 Normal 		: NORMAL;
    float2 UV           : TEXTURE;
};

struct VS_Buffer 
{
    matrix projectionMatrix;
    matrix viewMatrix;
    matrix modelMatrix;
};
ConstantBuffer<VS_Buffer> vs_buffer : register(b0, space0);

VS_OUTPUT main_VS(VS_INPUT input)
{
    float4 position = mul(vs_buffer.modelMatrix, float4(input.Position, 1.0));
    
    VS_OUTPUT output;
    output.UV = input.UV;
	output.Normal = input.Normal;
    output.Pos = mul(vs_buffer.projectionMatrix, mul(vs_buffer.viewMatrix, position));

    return output;
}

SamplerState colorSampler : register(s1, space0);
Texture2D colorTexture : register(t2, space0);

float4 main_FS(VS_OUTPUT input) : SV_TARGET0
{
    float4 outFragColor = colorTexture.Sample(colorSampler, input.UV);
    return outFragColor;
}