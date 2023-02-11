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
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};
ConstantBuffer<VS_Buffer> vs_buffer : register(b0, space0);

VS_OUTPUT main_VS(VS_INPUT input)
{
    float4 position = mul(vs_buffer.modelMatrix, float4(input.Position, 1.0));
    
    VS_OUTPUT output;
    output.UV = input.UV;
	output.Normal = mul((float3x3)vs_buffer.normalMatrix, input.Normal);
    output.Pos = mul(vs_buffer.projectionMatrix, mul(vs_buffer.viewMatrix, position));

    return output;
}

float4 main_FS(VS_OUTPUT input) : SV_TARGET0
{
    return float4(1.0, 1.0, 1.0, 1.0);
}