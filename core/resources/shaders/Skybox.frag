#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : enable

#include "Loader/SamplerCubeLoader.glsl"

layout (location = 0) in vec3 inUVW;

layout (push_constant) uniform PushConstant {
    uint skyboxTextureID;
};

layout (location = 0) out vec4 outBaseColor;

void main() 
{
    debugPrintfEXT("Hello from shader!\n");
    outBaseColor = SamplerCubeLoader_GetTexture(skyboxTextureID, inUVW);
}