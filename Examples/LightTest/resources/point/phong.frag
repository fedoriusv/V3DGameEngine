#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform texture2D textureDiffuse;
layout (binding = 3) uniform texture2D textureSpecular;

struct Light
{
    vec4 position;
    vec4 color;
};

layout (binding = 4) uniform LIGHT 
{
    Light lights[LIGHT_COUNT];
} light;

layout (binding = 5) uniform UBO 
{
    vec4 viewPosition;
} ubo;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 colorDiffuse = vec4(0.0);
    vec4 colorSpecular = vec4(0.0);
    
    vec3 normal = normalize(inNormal);
    vec4 colorTextureDiffuse = texture(sampler2D(textureDiffuse, samplerColor), inUV);
    vec3 viewDir = normalize(ubo.viewPosition.xyz - inPosition);
    for (int l = 0; l < LIGHT_COUNT; ++l)
    {
        vec3 lightPos = light.lights[l].position.xyz - inPosition;
        float distance = length(lightPos);
        float attenuation = 1.0 / (CONSTANT + LINEAR * distance + QUADRATIC * (distance * distance)); 
        
        vec3 lightDir = normalize(lightPos);
        float diffuseIntensive = max(dot(normal, lightDir), 0.01);
        colorDiffuse += light.lights[l].color * colorTextureDiffuse * diffuseIntensive * attenuation;
        
        if (diffuseIntensive > 0.01)
        {
            vec3 r = reflect(-lightDir, normal);
            vec4 colorTextureSpecular = texture(sampler2D(textureSpecular, samplerColor), inUV);
            colorSpecular += colorTextureSpecular * pow(max(dot(r, viewDir), 0.0), 8.0) * attenuation;
        }
    }
    
    outFragColor = colorDiffuse + colorSpecular;
}