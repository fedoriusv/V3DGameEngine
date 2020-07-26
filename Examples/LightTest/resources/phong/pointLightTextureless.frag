#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

struct Light
{
    vec4 position;
    vec4 color;
};

layout (binding = 1) uniform LIGHT 
{
    Light lights[LIGHT_COUNT];
} light;

layout (binding = 2) uniform UBO 
{
    vec4 viewPosition;
    vec4 diffuse;
    vec4 specular;  
} ubo;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 colorDiffuse = vec4(0.0);
    vec4 colorSpecular = vec4(0.0);
    
    vec3 normal = normalize(inNormal);
    for (int l = 0; l < LIGHT_COUNT; ++l)
    {
        vec3 lightPos = light.lights[l].position.xyz - inPosition;
        float distance = length(lightPos);
        float attenuation = 1.0 / (CONSTANT + LINEAR * distance + QUADRATIC * (distance * distance)); 
        
        vec3 lightDir = normalize(lightPos);
        float diffuseIntensive = max(dot(normal, lightDir), 0.01);
        colorDiffuse += ubo.diffuse * light.lights[l].color * diffuseIntensive;
        
        if (diffuseIntensive > 0.01)
        {
            float specPower = 8.0;
            vec3 r = reflect(-lightDir, normal);
            vec3 viewDir = normalize(ubo.viewPosition.xyz - inPosition);
            colorSpecular += ubo.specular * pow(max(dot(r, viewDir), 0.01), specPower) * attenuation;
        }
    }
    
    outFragColor = colorDiffuse + colorSpecular;
}