struct VS_ATTRIBUTES
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXTURE;
};

struct PS_ATTRIBUTES
{
    float4 Pos : SV_POSITION;
    
    float3 WorldPosition : POSITION;
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
    
    output.WorldPosition = position.xyz;
    output.Normal = mul(vs_buffer.normalMatrix, float4(input.Normal, 1.0)).xyz;
    output.UV = output.UV;
    
    output.Pos = mul(vs_buffer.projectionMatrix, mul(vs_buffer.viewMatrix, position));
    
    return output;
}

struct FS_Buffer
{
    float4 lightPosition;
    float4 viewPosition;
};
ConstantBuffer<FS_Buffer> fs_buffer : register(b1, space0);

SamplerComparisonState shadowSampler : register(s2, space0);
TextureCube shadowMapCube : register(t3, space0);

float shadowMask(float3 fragPos, float3 lightPos)
{
    float3 lightDirection = fragPos - lightPos; 
    float lightDistance = length(lightDirection) / (FAR_PLANE - NEAR_PLANE);
    
    float shadow = 1.0;
    float bias = 0.00;
    float depth = shadowMapCube.SampleCmpLevelZero(shadowSampler, lightDirection, lightDistance);
    if (depth < lightDistance - bias)
    {
        shadow = 0.2;
    }
    return shadow;
}

float4 main_FS(PS_ATTRIBUTES input) : SV_TARGET0
{
    float4 diffuseColor = float4(1.0, 1.0, 1.0, 1.0);
    float3 normal = normalize(input.Normal);
    float3 lightDirection = normalize(fs_buffer.lightPosition.xyz - input.WorldPosition);
    float diffuseKoeff = max(dot(normal, lightDirection), 0.01);
    float4 outFragColor = diffuseKoeff * diffuseColor;
    
    float shadow = shadowMask(input.WorldPosition, fs_buffer.lightPosition.xyz);
    
    if (diffuseKoeff > 0.01)
    {
        float4 specularColor = float4(1.0, 1.0, 1.0, 1.0);
        float3 lightReflectDirection = reflect(-lightDirection, normal);
        float3 viewDirection = normalize(fs_buffer.viewPosition.xyz - input.WorldPosition);
        outFragColor += specularColor * pow(max(dot(lightReflectDirection, viewDirection), 0.0), 512.0);
    }
    
    outFragColor = float4((outFragColor.xyz * shadow), 1.0);
    return outFragColor;
}