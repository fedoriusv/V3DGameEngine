struct VS_INPUT
{
    float3 Positions;
    float3 Normals;
    float3 Tangents;
    float3 Bitangents;
    float2 TexCoords;
};

struct VS_OUTPUT 
{
   float4 Pos: SV_POSITION;
   float3 Norm: NORMAL;
   float2 TexCoord: TEXCOORD0; 
};

cbuffer ConstantBuffer
{
    matrix projectionMatrix;
    matrix modelMatrix;
    matrix viewMatrix;
};

cbuffer ConstantBuffer2
{
    matrix projectionMatrix2;
    matrix modelMatrix2;
    matrix viewMatrix2;
};

VS_OUTPUT main(VS_INPUT Input)
{
   VS_OUTPUT Out;
   
   Out.Pos = mul(modelMatrix, float4(Input.Positions, 1.0));
   Out.Pos = mul(viewMatrix, Out.Pos);
   Out.Pos = mul(projectionMatrix2, Out.Pos);
   Out.TexCoord = Input.TexCoords;
   Out.Norm = Input.Normals;
   
   return Out;
}