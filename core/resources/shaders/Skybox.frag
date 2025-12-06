#version 450
#extension GL_GOOGLE_include_directive : require

#include "Loader/SamplerCubeLoader.glsl"

layout (location = 0) in vec3 inUVW;

layout (push_constant) uniform PushConstant {
    uint skyboxTextureID;
};

layout (location = 0) out vec4 outColor;

void main() 
{
    outColor = SamplerCubeLoader_GetTexture(skyboxTextureID, inUVW);
}