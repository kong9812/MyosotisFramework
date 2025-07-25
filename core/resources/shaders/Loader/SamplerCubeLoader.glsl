#ifndef SAMPLERCUBELOADER
#define SAMPLERCUBELOADER
#extension GL_EXT_nonuniform_qualifier : require

layout (set = 0, binding = 3) uniform samplerCube SamplerCube[];

vec4 SamplerCubeLoader_GetTexture(uint textureId, vec3 offset) {
    return texture(SamplerCube[textureId], offset);
}

#endif