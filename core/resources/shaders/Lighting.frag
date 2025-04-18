#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputPosition;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputNormal;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput inputBaseColor;
layout (binding = 3) uniform sampler2D shadowMap;
layout (binding = 4) uniform CameraUBO {
    vec4 position;
} cameraUbo;
layout (binding = 5) uniform LightUBO {
    mat4 viewProjection;
    vec4 position;
	int pcfCount;
} lightUbo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

float PCFShadow(vec3 shadowCoord, int pcfCount) 
{
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	uint count = 0; 
    for (int x = -pcfCount; x <= pcfCount; ++x) 
    {
        for (int y = -pcfCount; y <= pcfCount; ++y) 
        {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = texture(shadowMap, shadowCoord.xy + offset).r;
            shadow += (shadowCoord.z > closestDepth) ? 1.0 : 0.1;
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

const vec3 diffuseColor = vec3(1.0);
const vec3 ambientColor = vec3(1.0);
const vec3 specularColor = vec3(1.0);

void main() 
{
	vec4 position = subpassLoad(inputPosition);
    vec4 normal = subpassLoad(inputNormal);
	vec4 baseColor = subpassLoad(inputBaseColor);

	vec4 lightSpace = biasMat * lightUbo.viewProjection * position;
	float shadow = PCFShadow(vec3((lightSpace / lightSpace.w).xyz), lightUbo.pcfCount);

	vec3 lightDir = normalize(lightUbo.position.xyz - position.xyz);
    vec3 reflectDir = reflect(-lightDir, normal.xyz);
    vec3 viewDir = normalize(cameraUbo.position.xyz - position.xyz);
    vec3 halfwayDir = normalize(lightDir + cameraUbo.position.xyz);

    float diff = max(dot(normal.xyz, lightDir), 0.0);
    float spec = pow(max(dot(normal.xyz, halfwayDir), 0.0), 64.0);

	vec3 ambient = 0.5 * ambientColor;
    vec3 diffuse = diff * diffuseColor;
    vec3 specular = vec3(0.0);

	vec3 color = (ambient + (1.0 - shadow) * (diffuse + specular)) * baseColor.xyz;
	outColor = vec4(color, baseColor.a);
}