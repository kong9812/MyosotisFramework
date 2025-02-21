#version 450
// #extension GL_EXT_debug_printf : enable

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inBaseColor;
layout (location = 2) in vec4 inNormal;
layout (location = 3) in vec4 inCameraPosition;

layout (binding = 1) uniform sampler2D normalMap;
layout (binding = 2) uniform sampler2D shadowMap;
layout (binding = 3) uniform LightUBO {
    mat4 viewProjection;
    vec4 position;
	int pcfCount;
} lightUbo;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outBaseColor;

float PCFShadow(vec3 shadowCoord, int pcfCount) 
{
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	uint count = 0; 
    for (int x = -pcfCount; x <= pcfCount; ++x) {
        for (int y = -pcfCount; y <= pcfCount; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = texture(shadowMap, shadowCoord.xy + offset).r;
            shadow += (shadowCoord.z > closestDepth) ? 0.1 : 1.0;
			count++;
        }
    }
    return shadow / count;
}

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() 
{
	vec4 lightSpace = biasMat * lightUbo.viewProjection * inPosition;
    float shadow = PCFShadow(vec3((lightSpace / lightSpace.w).xyz), lightUbo.pcfCount);
    outPosition = inPosition;

    vec3 lightDir = normalize(lightUbo.position.xyz);
    float diff = max(dot(inNormal.xyz, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, inNormal.xyz);
    vec3 viewDir = normalize(inCameraPosition.xyz - inPosition.xyz);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 ambient  = 0.2 * inBaseColor.rgb;
    vec3 diffuse  = 0.1 * diff * inBaseColor.rgb * shadow;
    // vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    vec3 color = ambient + diffuse;
    
    // debugPrintfEXT("normal %f %f %f", inNormal.x, inNormal.y, inNormal.z);

    outNormal = inNormal;

    // vec3 color = inBaseColor.rgb * shadow;

    outBaseColor = vec4(color, inBaseColor.a);
    outColor = vec4(0.0);
}