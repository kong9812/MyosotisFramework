#ifndef SAMPLERCUBELOADER
#define SAMPLERCUBELOADER
#extension GL_EXT_nonuniform_qualifier : require

layout (binding = 2) uniform samplerCube SamplerCube[];

vec4 SamplerCubeLoader_GetTexture(uint textureId, vec3 offset)
{
    return texture(SamplerCube[textureId], offset);
}

#endif