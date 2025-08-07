#version 450
#extension GL_GOOGLE_include_directive : require

#include "Loader/SamplerCubeLoader.glsl"

layout (location = 0) in vec3 inUVW;
layout (location = 1) in flat uint inRenderID;

layout (binding = 2) uniform samplerCube SamplerCube[];
layout (push_constant) uniform PushConstant {
    uint StandardSSBOIndex;
    uint TextureId;
};

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outBaseColor;
layout (location = 3) out uint outRenderID;

void main() 
{
    outRenderID = inRenderID;
    outBaseColor = SamplerCubeLoader_GetTexture(TextureId, inUVW);
}