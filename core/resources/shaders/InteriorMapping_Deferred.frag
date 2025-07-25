#version 450
#extension GL_GOOGLE_include_directive : require

#include "Loader/SamplerCubeLoader.glsl"

layout (push_constant) uniform PushConstant {
    uint objectIndex;
    uint textureId;
};

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec4 inBaseColor;

layout (location = 3) in vec4 inRayDir;
layout (location = 4) in vec2 inUV;
layout (location = 5) in flat uint inRenderID;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outBaseColor;
layout (location = 3) out uint outRenderID;

void main() 
{    
    outPosition = inPosition;
    outNormal = inNormal;
    outRenderID = inRenderID;

    vec3 uvw = vec3(inUV.x, 1.0 - inUV.y, 0.0);
    vec3 rayDir = normalize(inRayDir.xyz);
    vec3 dist3 = (step(0.0, rayDir) - uvw) / rayDir;
    float dist = min(min(dist3.x, dist3.y), dist3.z);
    vec3 rayHit = uvw + rayDir * dist;
    vec3 offset = (rayHit - 0.5) * 2.0;

    outBaseColor = SamplerCubeLoader_GetTexture(textureId, offset);
}