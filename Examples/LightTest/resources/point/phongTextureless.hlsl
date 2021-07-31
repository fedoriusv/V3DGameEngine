struct VS_INPUT
{
    float3 Position  : IN_POSITION;
    float3 Normal    : IN_NORMAL;
    float3 Tangent   : IN_TANGENT;
    float3 Bitangent : IN_BITANGENT;     
    float2 UV        : IN_TEXTURE;
};

struct VS_UBO
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 normalMatrix;
    float4x4 viewMatrix;
};
ConstantBuffer<VS_UBO> vs_ubo : register(b0);

struct PS_INPUT
{
    float4 Pos       : SV_POSITION;
    
    float3 Position  : POSITION;
    float3 Normal    : NORMAL;    
    float2 UV        : TEXTURE;
};

PS_INPUT main_VS(VS_INPUT Input)
{
    PS_INPUT Output;
   
    float4 position = mul(vs_ubo.modelMatrix, float4(Input.Position, 1.0));
    Output.Pos = mul(vs_ubo.projectionMatrix, mul(vs_ubo.viewMatrix, position));
    Output.Position = position.xyz / position.w;
    Output.Normal = mul((float3x3)vs_ubo.normalMatrix, Input.Normal);
    Output.UV = Input.UV;

    return Output;
}

struct LIGHT
{
    float4 position;
    float4 color;
};

struct LIGHTS
{
    LIGHT lights[LIGHT_COUNT];
};
ConstantBuffer<LIGHTS> light : register(b1);

struct PS_UBO 
{
    float4 viewPosition;
    float4 diffuse;
    float4 specular;  
};
ConstantBuffer<PS_UBO> ps_ubo : register(b2);

float4 main_PS(PS_INPUT Input) : SV_TARGET0
{
    float4 colorDiffuse = float4(0.0, 0.0, 0.0, 1.0);
    float4 colorSpecular = float4(0.0, 0.0, 0.0, 1.0);
    
    float3 normal = normalize(Input.Normal);
    for (int l = 0; l < LIGHT_COUNT; ++l)
    {
        float3 lightPos = light.lights[l].position.xyz - Input.Position;
        float distance = length(lightPos);
        float attenuation = 1.0 / (CONSTANT + LINEAR * distance + QUADRATIC * (distance * distance)); 
        
        float3 lightDir = normalize(lightPos);
        float diffuseIntensive = max(dot(normal, lightDir), 0.01);
        colorDiffuse += ps_ubo.diffuse * light.lights[l].color * diffuseIntensive;
        
        if (diffuseIntensive > 0.01)
        {
            float3 r = reflect(-lightDir, normal);
            float3 viewDir = normalize(ps_ubo.viewPosition.xyz - Input.Position);
            colorSpecular += ps_ubo.specular * pow(max(dot(r, viewDir), 0.0), 8.0) * attenuation;
        }
    }
    
    float4 OutColor = colorDiffuse + colorSpecular;
    return OutColor;
}