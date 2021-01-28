struct VS_ATTRIBUTES
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXTURE;
};

struct PS_ATTRIBUTES
{
    float4 Pos : SV_POSITION;
    
    float3 PositionModel : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXTURE;
};

struct VS_Buffer 
{
    matrix projectionMatrix;
    matrix modelMatrix;
    matrix normalMatrix;
    matrix viewMatrix;
};
ConstantBuffer<VS_Buffer> vs_buffer : register(b0, space0);

PS_ATTRIBUTES main_VS(VS_ATTRIBUTES input)
{
    PS_ATTRIBUTES output;
    
    float4 position = mul(vs_buffer.modelMatrix, float4(input.Position, 1.0));
    
    output.PositionModel = position.xyz;
    output.Normal = mul(vs_buffer.normalMatrix, float4(input.Normal, 1.0)).xyz;
    output.UV = output.UV;
    
    output.Pos = mul(vs_buffer.projectionMatrix, mul(vs_buffer.viewMatrix, position));
    
    return output;
}

struct FS_Buffer
{
    float4 lightDirection;
    float4 viewPosition;
};
ConstantBuffer<FS_Buffer> fs_buffer : register(b1, space0);

float4 main_FS(PS_ATTRIBUTES input) : SV_TARGET0
{
    float4 diffuseColor = float4(1.0, 1.0, 1.0, 1.0);
    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(fs_buffer.lightDirection.xyz);
    float diffuseKoeff = max(dot(normal, lightDir), 0.01);
    float4 outFragColor = diffuseKoeff * diffuseColor;
    
    if (diffuseKoeff > 0.01)
    {
        float4 specularColor = float4(1.0, 1.0, 1.0, 1.0);
        float3 reflectDir = reflect(-lightDir, normal);
        float3 viewDir = normalize(fs_buffer.viewPosition.xyz - input.PositionModel);
        outFragColor += specularColor * pow(max(dot(reflectDir, viewDir), 0.0), 512.0);
    }
    
    return outFragColor;
}