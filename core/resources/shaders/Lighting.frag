#version 450
#extension GL_GOOGLE_include_directive : require

#include "Loader/MainCameraDataLoader.glsl"
#include "Loader/Sampler2DLoader.glsl"
#include "SSBO/DirectionalLightSSBO.glsl"

layout (set = 1, input_attachment_index = 0, binding = 0) uniform subpassInput inputPosition;
layout (set = 1, input_attachment_index = 1, binding = 1) uniform subpassInput inputNormal;
layout (set = 1, input_attachment_index = 2, binding = 2) uniform subpassInput inputBaseColor;

layout (push_constant) uniform PushConstant {
    uint objectIndex;
    uint textureId;
};

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

float PCFShadow(vec3 shadowCoord, int pcfCount) 
{
    float shadow = 0.0;
    vec2 texelSize = 1.0 / Sampler2DLoader_GetTextureSize(textureId, 0);
	uint count = 0; 
    for (int x = -pcfCount; x <= pcfCount; ++x) 
    {
        for (int y = -pcfCount; y <= pcfCount; ++y) 
        {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = Sampler2DLoader_GetTexture(textureId, shadowCoord.xy + offset).r;
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
    RawDataMetaData meta = RawDataLoader_GetRawDataMetaData(objectIndex);
    MainCameraData cameraData = MainCameraDataLoader_GetMainCameraData();
    DirectionalLightSSBO directionalLightSSBO = DirectionalLightSSBO_LoadDirectionalLightSSBO(meta.dataOffset + 0);

	vec4 position = subpassLoad(inputPosition);
    vec4 normal = subpassLoad(inputNormal);
	vec4 baseColor = subpassLoad(inputBaseColor);

	vec4 lightSpace = biasMat * directionalLightSSBO.viewProjection * position;
	float shadow = PCFShadow(vec3((lightSpace / lightSpace.w).xyz), directionalLightSSBO.pcfCount);

	vec3 lightDir = normalize(directionalLightSSBO.position.xyz - position.xyz);
    vec3 reflectDir = reflect(-lightDir, normal.xyz);
    vec3 viewDir = normalize(cameraData.position.xyz - position.xyz);
    vec3 halfwayDir = normalize(lightDir + cameraData.position.xyz);

    float diff = max(dot(normal.xyz, lightDir), 0.0);
    float spec = pow(max(dot(normal.xyz, halfwayDir), 0.0), 64.0);

	vec3 ambient = 0.5 * ambientColor;
    vec3 diffuse = diff * diffuseColor;
    vec3 specular = vec3(0.0);

	vec3 color = (ambient + (1.0 - shadow) * (diffuse + specular)) * baseColor.xyz;
	outColor = vec4(color, baseColor.a);
}