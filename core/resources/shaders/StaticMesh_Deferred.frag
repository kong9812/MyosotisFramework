#version 450

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inBaseColor;
layout (location = 2) in vec3 inNormal;

layout (binding = 1) uniform sampler2D normalMap;
layout (binding = 2) uniform sampler2D shadowMap;
layout (binding = 3) uniform LightUBO {
    mat4 viewProjection;
    vec4 position;
	int pcfCount;
} lightUbo;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outBaseColor;

float hardShadow(vec4 lightSpacePosition, vec2 offsetPosition)
{
	float shadow = 1.0;
	if ( lightSpacePosition.z > 0.0 && lightSpacePosition.z < 1.0 ) 
	{
		float dist = texture( shadowMap, lightSpacePosition.st + offsetPosition).r;
		if (lightSpacePosition.w > 0.0 && dist < lightSpacePosition.z) 
		{
			shadow = 0.1;
		}
	}
	return shadow;
}

float softShadow(vec4 lightSpacePosition, int pcfCount)
{
	ivec2 shadowMapTextureSize = textureSize(shadowMap, 0);
	float dx = 1.0 / shadowMapTextureSize.x;
	float dy = 1.0 / shadowMapTextureSize.y;
	float shadow = 1.0;
	int count = 0;
	for (int x = -pcfCount; x <= pcfCount; x++)
	{
		for (int y = -pcfCount; y <= pcfCount; y++)
		{
			shadow += hardShadow(lightSpacePosition, vec2(dx*x, dy*y));
			count++;
		}
	}
	return shadow / count;
}

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
    vec3 color = inBaseColor.rgb * shadow;

    outBaseColor = vec4(color, inBaseColor.a);
    outColor = vec4(0.0);
}