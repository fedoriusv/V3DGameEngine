#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inLightPos;
layout (location = 3) in vec3 inViewDir;
layout (location = 4) in vec2 inUV;

layout (binding = 3) uniform samplerCube shadowSampler;
layout (binding = 4) uniform textureCube shadowCubeMap;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 diffuseColor = vec4(1.0);
    vec3 normal = normalize(inNormal);
    vec3 lightDir = normalize(inLightPos - inPosition);
    float diffuseKoeff = max(dot(normal, lightDir), 0.01);
    outFragColor = diffuseKoeff * diffuseColor;

    if (diffuseKoeff > 0.01)
    {
        vec4 specularColor = vec4(1.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 viewDir = normalize(inViewDir);
        outFragColor += specularColor * pow(max(dot(reflectDir, viewDir), 0.0), 512.0);
    }
    
    // Shadow
	//vec3 lightCoord = inPosition - inLightPos;
    //float sampledDist = texture(samplerCube(shadowCubeMap, shadowSampler), lightCoord).r;
    //float dist = length(lightCoord);

	// Check if fragment is in shadow
    //float shadow = (dist <= sampledDist + EPSILON) ? 1.0 : SHADOW_OPACITY;
    		
	//outFragColor.rgb *= shadow;
    
    //float shadow = shadowCalculation(inLightSpace); 
    //outFragColor = shadow * outFragColor;
}